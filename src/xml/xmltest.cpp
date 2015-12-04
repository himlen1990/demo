#include "tinyxml.h"
#include <iostream>
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <face_recognition/FaceRecognitionAction.h>
#include <face_recognition/FRClientGoal.h>


using namespace std;
const char* filename("demo.xml");
bool query_flag(true);


/*void creat_new_file(TiXmlDocument &doc, const char* filename);
TiXmlElement* find_elem(TiXmlElement* &parent, const string name);
TiXmlElement* add_new_elem(TiXmlElement* &parent, const char* elem_name, const char* elem_content);
void load_person_db(TiXmlElement* person, map<string,string>  &item_map);//for multi item, multimap is better*/

/* 1. found xml file
   2. accept qurey from someone
   3. found that person in xml then load db (if not found, creat one)
   4. if new person, ask , the creat db.
*/
class item_query
{
  ros::NodeHandle nh_;
  ros::Subscriber sub;
  ros::Publisher query_result_pub;



public:
  face_recognition::FRClientGoal msg;
  ros::Publisher face_rec_pub;
  
  item_query()
  {
    sub = nh_.subscribe("face_recognition/result",1,&item_query::search_person_db, this);
    face_rec_pub = nh_.advertise<face_recognition::FRClientGoal>("fr_order", 1);
    query_result_pub=nh_.advertise<std_msgs::String>("query_result",1);
    msg.order_id=0;
    msg.order_argument="none";

  }
  
  ~item_query()
  {}


  TiXmlElement* find_elem(TiXmlElement* &parent, const string name)
  {
    TiXmlElement *elem = parent-> FirstChildElement();
    while(elem)
      {
        if (!name.compare(elem->FirstChild()->Value()))
          {
            return elem;
          }
        
        elem = elem->NextSiblingElement();
      }
    return NULL;

  }

TiXmlElement*
add_new_elem(TiXmlElement* &parent, const char* elem_name, const char* elem_content){
  TiXmlElement *new_elem = new TiXmlElement(elem_name);
  TiXmlText* text = new TiXmlText(elem_content);
  new_elem->LinkEndChild(text);
  parent->LinkEndChild(new_elem);
    return new_elem;
  }

void 
creat_new_file(TiXmlDocument &doc, const char* filename){

  TiXmlDeclaration *decl = new TiXmlDeclaration("1.0","UTF-8","yes");
  TiXmlElement* person_db= new TiXmlElement("person_db");
    doc.LinkEndChild(decl);
    doc.LinkEndChild(person_db);
    doc.SaveFile(filename);
  
  }

void load_person_db(TiXmlElement* person, map<string, string> &item_map)
{

    TiXmlElement* item = person->FirstChildElement();
    while (item) {
        string item_name = item->Value();
        string item_content = item->FirstChild()->Value();
        item_map.insert(pair<string,string> (item_name,item_content));
        item = item->NextSiblingElement();
    }
}

void search_person_db(const face_recognition::FaceRecognitionActionResult &msg)
{

  string  item("drink");
  string query;
  char person_elem[]="person";
  TiXmlDocument doc;
  //string st1;
  string st1=msg.result.names[0].c_str();
   

    map<string,string>  item_map;
    const char *person_name = st1.c_str();
    map<string,string>::iterator  item_map_it;

    
  if(!doc.LoadFile(filename))
    {
      creat_new_file(doc, filename);
    }
    
    TiXmlElement *root = doc.RootElement();
    TiXmlElement* person = find_elem(root, st1);
    if (person) {
        load_person_db(person,item_map);

        //istringstream ss(query);    
        istringstream ss("drink");    
        while(getline(ss,item,' '))
          {
            item_map_it=item_map.find(item);
            if(item_map_it!=item_map.end()){
              cout<<"I think your favorite "<< item << " is " << item_map_it->second  <<endl;
              std_msgs::String query_result;
              query_result.data=item_map_it->second;
              query_result_pub.publish(query_result);
              query_flag=true;
            }
            else
              cout<<"cannot find such object in database"<< endl;
          }
    }
    else  {
        person = add_new_elem(root,person_elem,person_name);
        cout<< "add person: " << st1 << endl;
        cout<< "we have georgia, wonda and iemon ,which one do you like?" << endl;
        getline(cin,query);
        while((query.compare("georgia"))&&(query.compare("wonda"))&&(query.compare("iemon")))
          {
            cout<<"please type one item among the choice"<< endl;
            getline(cin,query);
          }
        add_new_elem(person, item.c_str(), query.c_str());
        doc.SaveFile(filename);
        cout<< "your choice has writtern to database" << endl;
        cout<<"done"<<endl;
        std_msgs::String query_result;
        query_result.data=query.c_str();
        query_result_pub.publish(query_result);
        query_flag=true;
          
            
          
    }

}
};


int main(int argc, char** argv){
  ros::init(argc,argv,"person_db");
  item_query iq;
  string query;
  ros::Rate loop_rate(0.1);
  cout << "would you like some drinks? (yes/no)" << endl;

  getline(cin,query);
  if(!query.compare("yes")){
    iq.face_rec_pub.publish(iq.msg);
    query_flag=false;
    cout<< "waiting for recognition result"<< endl;
        }
  while(ros::ok()){   
   
    ros::spinOnce();

    loop_rate.sleep();
    }
  return 0;
}
