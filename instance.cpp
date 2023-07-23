#include <iostream>
#include "instance.h"
#include <fstream>
#include <regex>
#include <algorithm>
#include <string>
#include <cmath>

#define PARAMETERS_CHECK 6      

/***********************************************************************************
 *  CLASS: Instance
 *
 *  @brief Read an instance file and store its data
 *
 *  @author Bruno Bruck
 *  @version 1.0.0
 *  @date August 22, 2022
 ***********************************************************************************/


/***********************************************************************************
 *  Constructor
 *
 *  @date August 15, 2022
 ********************************/
Instance::Instance(){
    init();
}


/***********************************************************************************
 *  Destructor
 *
 *  @date August 16, 2022
 ********************************/
Instance::~Instance(){
    destroy();
}

//the correct way to run the instances: ./pdprings -i instances/drectory/.txt
/***********************************************************************************
 *  load the instance
 *
 *  @brief Load instance data from single ring instances
 *
 *  @param instance_path (string): The path to the instance
 *
 *  @return [bool] In case everything went ok return true, false otherwise
 *  @date August 22, 2022
 ********************************/
bool Instance::loadInstance(string instance_path){
    // Clear the object in case it there was already an instance loaded
    if(loaded_)
        clear();
    
    //cout << "this is the instance path " << instance_path << endl; 
    this->instance_path_ = instance_path;

    // Try to open the instance
    ifstream instance_file;
    instance_file.open(instance_path_.c_str());
    if(!instance_file){
        //cerr << utils::clr::red
            cout << "## Error: Could not open instance file (at Instance::load, line"; 
          //   << utils::clr::none;
        return false;
    }

    string aux;

    // Read name, dimension_, capacity, vehicles
    instance_file >> aux >> name_instance
                  >> aux >> dimension
                  >> aux >> capacity
                  >> aux >> vehicles;

    
    // Read the data
    instance_file.ignore(100000, '\n');    //just ignoring the rest of the line 

    vector<string> correct_pattern = {"No", "x_pos", "y_pos", "demand", "release", "due"};    //defining the vector with right pattern

    int counter = 1;
    while (counter != PARAMETERS_CHECK){       //checking de first line: "No x_pos y_pos demand release  due"
        
        instance_file >> aux;

        if(aux != correct_pattern[counter-1]){ 
           // cout << utils::clr::red
            cout << "## Error: Instance format incorrect, detected " << aux << ", but it was expected " << correct_pattern[counter-1];
            //<< " (at Instance::load, line " << __LINE__ << ")" << endl
            //<< utils::clr::none;
            return false;
        }

        counter++;
    }

    instance_file.ignore(100000, '\n'); // Ignore the rest of the current line

    
    int current_position = 0; // Counter 
    

    while(true){

        getline(instance_file, aux);
        
        std::regex pattern("\\s+");
        aux = std::regex_replace(aux, pattern, " ");         //to substitue the irregular spaces for just one space

        
        // Exit flag
        if(instance_file.eof()) 
            break;

        // Read the current line
        //vector<string> values = utils::split(aux, ' ');

        
        vector<string> attributes_i = treating_string(aux);

        if(attributes_i.size() != 6){
            //cout << utils::clr::red
            std::cout << "## Error: Instance format incorrect, unexpected value \"" << aux << "\" while reading the data (at Instance::load, line ";// << __LINE__ << ")" << endl
            // << utils::clr::none;
            return false;
        }

        Request* new_request = new Request();   //new object request to get the data
        new_request->no = std::stoi(attributes_i[0]);
        new_request->x_pos = std::stod(attributes_i[1]);
        new_request->y_pos = std::stod(attributes_i[2]);
        new_request->demand = std::stoi(attributes_i[3]);
        new_request->release_date = std::stoi(attributes_i[4]);
        new_request->due_date = std::stoi(attributes_i[5]);

        requests_.push_back(new_request);            //adding the new request to the vector with all 

        current_position++;
    }    

    //defining the matrix of distances

    c_distances.resize(dimension);

    for (auto& row : c_distances) {
        row.resize(dimension);
    }
    
    // Set the diagonal of the distance matrix as 0

    for(int i = 0 ; i < dimension ; i++){
        for(int j = 0 ; j < dimension ; j++){
            
            if(i == j){
                c_distances[i][j] = 0;

            }else{
                c_distances[i][j] = round(CalcDistEucledian(requests_[i]->x_pos, requests_[j]->x_pos, requests_[i]->y_pos,requests_[j]->y_pos));           
                
                //rounding the distance from vertice i to j 
            }
        }
    }
        
    return true;
}

/***********************************************************************************
 *  CalcDistEucledian
 *
 *  @brief calculate the distance between two points
 
 ********************************/

double Instance::CalcDistEucledian (double &x_1, double &x_2, double &y_1, double &y_2){

    return sqrt ( pow ( x_1 - x_2, 2 ) + pow ( y_1 - y_2, 2 ) );
}   


/***********************************************************************************
 *  init
 *
 *  @brief Initialize the attributes
 *
 *  @date August 22, 2022
 ********************************/
void Instance::init(){
    loaded_ = false;
    
    name_instance = "";
    dimension = 0;
    capacity = 0;
    vehicles = 0;
}


/***********************************************************************************
 *  destroy
 *
 *  @brief Free allocated memory for the attributes of the object
 *
 *  @date August 22, 2022
 ********************************/
void Instance::destroy(){
    //size_t reqsize = requests_.size();
    //for(unsigned int i = 0 ; i < reqsize ; i++)
     //  delete requests_[i];
}


/***********************************************************************************
 *  clear
 *
 *  @brief Clear the object so it can be loaded again
 *
 *  @date October 22, 2019
 ********************************/
void Instance::clear(){
    destroy();

    instance_path_ = "";

    requests_.clear();

    init();
}

/***********************************************************************************
 *  print
 *
 *  @brief Print the instance
 *
 *  @date August 22, 2022
 ********************************/
void Instance::print(){
    cout << ">> Printing instance..." << endl;

    cout << "Name of the instance = " << name_instance << endl;
    cout << "Dimension = " << dimension << endl;
    cout << "Vehicle capacity = " << capacity << endl;
    cout << "Number of vehicles = " << vehicles << endl;
    

    // Print the details of each request
    for(unsigned int i = 0 ; i < requests_.size() ; i++){
        cout << "============================  Request " << i << " ============================" << endl
             << "   No: " << requests_[i]->no << endl
             << "   x_pos: " << requests_[i]->x_pos << endl 
             << "   y_pos: " << requests_[i]->y_pos << endl 
             << "   demand: " << requests_[i]->demand << endl 
             << "   release_date: " << requests_[i]->release_date << endl 
             << "   due_date: " << requests_[i]->due_date << endl;
             //getchar();

    }

    cout << endl;
    cout << "Distance matrix:" << endl << setprecision(2) << fixed;

    
    for(int i = 0 ; i < dimension ; i++){
        cout << "Node " << i << " distances: " << endl;
        for(int j = 0 ; j < dimension ; j++){       
            //cout << "[" << j << "]: " <<  c_distances[i][j]  << "   ";
            cout <<  c_distances[i][j]  << ", ";
            //if(j == dimension/2){
            //    cout << endl;
            //}
        }
        cout << endl << endl;
    }
        
    cout << "===============================================================" << endl;
}


/***********************************************************************************
 *  function to treat the string

 ********************************/

vector<string> Instance::treating_string(string to_edit){

    if (to_edit.back() == ' ') {     //removing the space in the final of string
            to_edit.pop_back();
    }
    
    vector<std::string> attributes;
    std::string::size_type start = 0;
    string::size_type end = 0;

    while ((end = to_edit.find(' ', start)) != std::string::npos) {    //localizing the spaces and removing them 
        attributes.push_back(to_edit.substr(start, end - start));
        start = end + 1;
    }
    attributes.push_back(to_edit.substr(start));          

    return attributes;
}