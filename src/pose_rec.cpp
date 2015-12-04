#include <ros/ros.h>
#include <tf/transform_listener.h>
#include <geometry_msgs/Twist.h>
#include <vector>
#include <fstream>
using namespace std;

const char* file_name="pose_train.dat";
int vector_num=0;

double v_norm(vector<double> &v)
{
  double sum=0.0;
  for(int i=0; i<v.size(); i++){
    sum+=v[i]*v[i];
  }
  return sqrt(sum);
}

vector<double> v_sub(tf::StampedTransform transform1, tf::StampedTransform transform2)
{
  vector<double> v;
  v.push_back(transform1.getOrigin().x()-transform2.getOrigin().x());
  v.push_back(transform1.getOrigin().y()-transform2.getOrigin().y());
  v.push_back(transform1.getOrigin().z()-transform2.getOrigin().z());
  return v;
}

int main(int argc, char** argv)
{
  ros::init(argc, argv, "pose_descriptor_extractor");
  ros::NodeHandle nh_;
  tf::TransformListener listener;
  ros::Rate rate(10.0);
  fstream fout;
  fout.open(file_name,ios::app|ios::out);
  if(!fout.is_open()){
    cerr <<"cannot open file"<< endl;
    return false;
  }
  tf::StampedTransform transform;
  vector<tf::StampedTransform> transform_v;
  bool new_msg_flat=false;
  transform_v.resize(9);
  string frame_name[9]={"/head_1","/neck_1","/torso_1","/left_shoulder_1","/left_elbow_1","/left_hand_1","/right_shoulder_1","/right_elbow_1","/right_hand_1"};

  
  while(nh_.ok())
    {
      try{
        for(int i=0; i<8; i++){
           listener.lookupTransform("/openni_depth_frame", frame_name[i] ,ros::Time(0) ,transform);

          transform_v[i]=transform;

        }
        new_msg_flat=true;
      }
      catch (tf::TransformException ex)
        {
          
          ros::Duration(0.1).sleep();
  
        }
      if(new_msg_flat==true)
        {

          vector<double> d_l = v_sub(transform_v[5],transform_v[4]);
          vector<double> d_r = v_sub(transform_v[8],transform_v[7]);      

          double ref_dis=max(v_norm(d_l), v_norm(d_r)); //use the ||left/right hand - elbow|| as reference distance
          //        cout<<"distance"<<ref_dis<<endl;
          vector<vector<double> > p(transform_v.size());
          for(int i=0; i<transform_v.size(); i++)
            {

              
              vector<double> temp=(v_sub(transform_v[i],transform_v[2])); // every joint relative to head
              for(int j=0; j<temp.size(); j++)
                {
                  p[i].push_back(temp[j]/ref_dis);
                  fout<< p[i][j]<<",";
                }
              
              
            }

          fout<<endl;
          vector_num++;
          //listener.clear();       
          new_msg_flat==false;
        }
     
      rate.sleep();
    }
  cout<<vector_num<<endl;
  fout.close();
  return 0;
}


