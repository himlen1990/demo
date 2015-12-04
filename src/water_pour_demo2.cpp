/*
  how it works
  1. find the desk using color filter (the color of the desk is already known
  2. find the contour of desk (it should be the largest contour)
  3. creat a bounding box of the desk(largest contour)
  4. make the bounding box smaller, so that we can detect the cup easier
  5. find the cup using houghcircle
  6. vote and decide the mid point of the cup
*/

#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/flann/flann.hpp>
#include <fstream>
#include <math.h>
#include <iostream>
#include <geometry_msgs/PointStamped.h>

static const std::string OPENCV_WINDOW = "Image window";

using namespace std;
using namespace cv;

class ImageConverter
{
  ros::NodeHandle nh_;
  image_transport::ImageTransport it_;
  image_transport::Subscriber image_sub_;
  ros::Publisher  center_pub_;
  Point selected_center;
  int center_vote,selected_radius;
  Mat descriptor_object;
  const char* surfFile; 
  const char* siftFile;
  double max_dist ;
  double min_dist ;
  //  int image_count;
  int pub_counter;
public:
  ImageConverter()
    : it_(nh_)
  {
    selected_center=Point (-1,-1);
    selected_radius=-1;
    center_vote=0;
    //----descripor matching distance
    max_dist =0;
    min_dist = 100;
    //  image_count=0;
    image_sub_ = it_.subscribe("/openni_c2/rgb/image_rect_color", 1, 
                               &ImageConverter::desk_detect, this);
    //image_sub_ = it_.subscribe("/camera/rgb/image_rect_color", 1, 
    //                           &ImageConverter::desk_detect, this);

    // surfFile = "/home/himlen/Desktop/descriptor.txt"; 
    siftFile = "/home/himlen/Desktop/sift_descriptor.txt"; 
    load_data(siftFile, descriptor_object); 
    center_pub_ = nh_.advertise<geometry_msgs::PointStamped>("cup_center", 100);

    cv::namedWindow(OPENCV_WINDOW,0);
  }

  ~ImageConverter()
  {
    cv::destroyWindow(OPENCV_WINDOW);
  }

  bool
  load_data(const char* filename,  Mat& descriptor){
    const int DIM_SURF=64,DIM_SIFT=128;
    ifstream fin;
    fin.open(filename, ios::in);
    if(!fin.is_open()){
      cerr << "cannnot open file" << endl;
      return false ;
    }
    int numkeypoint = 0;
    string line;

    while(getline(fin, line, '\n')){
      numkeypoint++;
    }

    descriptor.create(numkeypoint, DIM_SIFT, CV_32FC1);

    fin.clear();
    fin.seekg(0);
  
    int row=0;
    while(getline(fin, line, '\n')){
      vector<string> ldata;
      istringstream ss(line);
      string s;
      float *rowPtr;
      rowPtr = descriptor.ptr<float>(row);
      while (getline(ss, s, ' ')) {
        ldata.push_back(s);
      }
      for(int i=0 ; i<DIM_SIFT; i++){
        float val= atof(ldata[i+3].c_str());    
        rowPtr[i]= val;
      }
      row++;
    }
    fin.close();
    return true;
  }



  void desk_detect(const sensor_msgs::ImageConstPtr& msg)
  {
    
    cv_bridge::CvImagePtr cv_ptr;
    try
      {
        cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
      }
    catch (cv_bridge::Exception& e)
      {
        ROS_ERROR("cv_bridge exception: %s", e.what());
        return;
      }
    RNG rng(12345);

    //surf setting----------------------------------------
    int minHessian = 400;
    int octaves = 3;
    int octaveLayers = 4;
    // SurfFeatureDetector detector(minHessian, octaves, octaveLayers);
    // SurfDescriptorExtractor extractor;

    SiftFeatureDetector detector;
    SiftDescriptorExtractor extractor;
    vector<KeyPoint> keypoint_scene;
    Mat descriptor_scene;
    FlannBasedMatcher matcher;
    vector<DMatch> matches;


    //color filter setting-----------------------------------
    Mat hsl, desk_roi, desk_bin;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    cvtColor(cv_ptr->image,hsl, CV_RGB2HLS); 
    // double min_h=10, max_h=160, min_s=200, max_s=255, min_l=5, max_l=202;
    double min_h=10, max_h=160, min_s=150, max_s=255, min_l=5, max_l=202;
    Scalar hsl_lower(min_h, min_s, min_l);
    Scalar hsl_upper(max_h, max_s, max_l);
    inRange(hsl, hsl_lower, hsl_upper, desk_bin);
    
    namedWindow("desk_bin",0);
    imshow("desk_bin", desk_bin);
    //find the desk---------------------------------------
    findContours(desk_bin ,contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

    int biggestContourIdx = -1;
    float biggestContourArea = 0;
    for( int i = 0; i< contours.size(); i++ )
      {
        
        float ctArea= cv::contourArea(contours[i]);
        if(ctArea > biggestContourArea)
          {
            biggestContourArea = ctArea;
            biggestContourIdx = i;
          }
      }
   

    vector<Point>  max_contours_poly;
    Rect max_boundRect;
    if(contours.size()>0){
      approxPolyDP( Mat(contours[biggestContourIdx]), max_contours_poly, 3, true );
      max_boundRect = boundingRect( Mat(max_contours_poly) );
    }
    
    //crop  ROI of  desk
    if(max_boundRect.width>50&&max_boundRect.height>50) //all following code will not execute if we can't find boundRect
      {
        Mat desk_gray, desk_canny;
        desk_roi=cv_ptr->image(max_boundRect);
        cvtColor( desk_roi, desk_gray, CV_BGR2GRAY);
        blur(desk_gray, desk_gray, Size(3,3));
        Canny(desk_gray, desk_canny, 90 ,250 ,3);
        

        //---------------get desk roi-------------------------------                                
        
        vector <Vec3f> circles;
        HoughCircles(desk_canny, circles, CV_HOUGH_GRADIENT,2,desk_canny.rows/4, 200, 50 , 0 ,desk_canny.rows/4 );
        
        
          //-----------------------------match object in desk using loaded data---------                      
          Mat desk_roi_gray;
          cvtColor(desk_roi, desk_roi_gray, CV_BGR2GRAY);
          detector.detect(desk_roi_gray, keypoint_scene);
          extractor.compute(desk_roi_gray, keypoint_scene, descriptor_scene);

          matcher.match(descriptor_object, descriptor_scene, matches);
          
        for (int i=0; i<descriptor_object.rows; i++){
          double dist = matches[i].distance;
          if (dist < min_dist) min_dist = dist;
          if (dist > max_dist) max_dist = dist;
        }
        
    
        vector<DMatch> good_matches;
        for (int i=0; i<descriptor_object.rows; i++){
          if (matches[i].distance < 2.5*min_dist) {
            good_matches.push_back(matches[i]);
          }
        }
        cout<<"match size  "<<good_matches.size() <<endl;
        Mat img_matches;
  
        vector<Point>scene;
        for(int i=0; i<good_matches.size(); i++){
          scene.push_back(keypoint_scene[good_matches[i].trainIdx].pt);

        }
        
        float min_dis=100;
        float min_dis_idx=0;

        vector<Rect> boundRect;
        for(int i=0; i<circles.size(); i++){
            Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
            int radius = cvRound(circles[i][2]);
            Rect temp_rect(center.x-radius,center.y-radius,3*radius,5*radius);
            boundRect.push_back(temp_rect);
            }

       
        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
        /*for(int i=0 ; i<boundRect.size(); i++)
        rectangle( desk_roi, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );*/
          
        vector<int> vote_box(boundRect.size());
        for(int i=0; i<scene.size(); i++){
          for(int j=0; j<vote_box.size(); j++){
            if(scene[i].inside(boundRect[j]))
              vote_box[j]++;
          }
          circle(desk_roi, scene[i], 5 , Scalar(140), 2 );
        }

        
        int most_vote=0,most_vote_id=0;
        for(int i=0; i<vote_box.size(); i++){
          if(vote_box[i]>most_vote){
            most_vote=vote_box[i];
            most_vote_id=i;
          }
        }
        
        if(boundRect.size()>0){
          rectangle( desk_roi, boundRect[most_vote_id].tl(), boundRect[most_vote_id].br(), color, 2, 8, 0 );
        
          Point cup_center(cvRound(circles[most_vote_id][0]), cvRound(circles[most_vote_id][1]));
          int cup_radius = cvRound(circles[most_vote_id][2]);
          
          if(this->selected_center== Point (-1,-1)||this->center_vote==0)
            {
              selected_center=cup_center;
              selected_radius=cup_radius;
              pub_counter=0;
            }
            
          if(
             this->center_vote<=10&&
             ((abs(this->selected_center.x-cup_center.x))<=this->selected_radius/2)&&
             ((abs(this->selected_center.y-cup_center.y))<=this->selected_radius/2)&&
             ((abs(this->selected_radius-cup_radius))<=5))
              
            this->center_vote++;
            
          
          if(
             (abs(cup_center.x-this->selected_center.x)>this->selected_radius/2)||
             (abs(cup_center.y-this->selected_center.y)>this->selected_radius/2)||
             ((abs(this->selected_radius-cup_radius))>5)&&
             (this->selected_center!= Point (-1,-1)))
            {
              this->center_vote--;
              if(this->center_vote<0)
                this->center_vote=0;
            }
        
          Point cup_center_tran(this->selected_center.x+max_boundRect.x, this->selected_center.y+max_boundRect.y);
        
          circle(cv_ptr->image , cup_center_tran, 3 , Scalar(0,255,0),3 ,8, 0);

          cout<<"vote "<< center_vote<<endl;

          circle( desk_roi, selected_center , selected_radius, Scalar(0,0,255), 3, 8, 0 );
          namedWindow("SRC",0);
          imshow("SRC",desk_roi);

          if((center_vote>5)&&(pub_counter<3)){
            geometry_msgs::PointStamped center;
            center.point.x=cup_center_tran.x;
            center.point.y=cup_center_tran.y;
            center.point.z=0;
            center.header.stamp = msg->header.stamp;
            center_pub_.publish(center);
          pub_counter++;
          }
        }
                   
      }

    imshow(OPENCV_WINDOW, cv_ptr->image);
    //    char filename[100];
    //sprintf(filename,"/home/himlen/Desktop/cup_image/result%d.jpg",image_count);
    //imwrite(filename,cv_ptr->image);
    //image_count++;
    waitKey(10);
      }
};

int main(int argc, char** argv)
{

  
  
 
  ros::init(argc, argv, "image_converter");
  ImageConverter ic;
  
  ros::Rate loop_rate(4);
  while(ros::ok())
    {
      ros::spinOnce();
      loop_rate.sleep();
    }
  return 0;
}
