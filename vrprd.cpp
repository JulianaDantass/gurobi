#include <gurobi_c++.h>
#include <iostream>
#include "classes.h"
#include <vector>
#include <fstream>
#include <regex>
#include <algorithm>
#include <string>
#include <cmath>

#define PARAMETERS_CHECK 6

class Instance{
    public: // Public attributes
        bool loaded_;

        string name_instance;

        int  dimension
            ,capacity
            ,vehicles;

        string instance_path_;

        vector<Request*> requests_;        //it has also the vertice 0 
        vector<vector<double>> c_distances;
        

    public: // Public methods

        double CalcDistEucledian(double &x_1, double &x_2, double &y_1, double &y_2){
            return sqrt ( pow ( x_1 - x_2, 2 ) + pow ( y_1 - y_2, 2 ) );
        }

        bool loadInstance(string instance_path){
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

        vector<string> treating_string(string to_edit){
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
};

void solve(Instance current_instance){

    try {
        // Cria o ambiente Gurobi
        GRBEnv env = GRBEnv();
        env.set(GRB_IntParam_OutputFlag, 0); // Desativa a saída no console do Gurobi

        // Cria o modelo
        GRBModel modelo = GRBModel(env);

        // Cria as variáveis de decisão
        char var[100];                        
    
        GRBVar*** x = new GRBVar**[current_instance.dimension];

        for (int i = 0; i < current_instance.dimension; i++) {
            x[i] = new GRBVar*[current_instance.dimension];

            for (int j = 0; j < current_instance.dimension; j++) {
                x[i][j] = new GRBVar[current_instance.vehicles];
                
                for (int k = 0; k < current_instance.vehicles; k++) {

                    if (i != j){
                        sprintf(var, "x(%d,%d,%d)", i, j, k);
                        x[i][j][k] = modelo.addVar(0.0, 1.0, 0.0, GRB_BINARY, var);
                    }
                }
            }
        }

        GRBVar** t = new GRBVar*[current_instance.dimension];

        for (int i = 0; i < current_instance.dimension; i++) {
            t[i] = new GRBVar[current_instance.vehicles];
            for (int j = 0; j < current_instance.vehicles; j++) {

                    sprintf(var, "t(%d,%d)", i, j);
                    t[i][j] =  modelo.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, var);
            }
        }

        GRBVar** h = new GRBVar*[current_instance.dimension];

        for (int i = 1; i < current_instance.dimension; i++) {
            h[i] = new GRBVar[current_instance.vehicles];
            for (int j = 0; j < current_instance.vehicles; j++) {

                    sprintf(var, "h(%d,%d)", i, j);
                    h[i][j] =  modelo.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, var);
                }
                
            
        }
    
        GRBLinExpr sum = 0;
        for(int j = 0 ; j < current_instance.dimension; j++){        //first somatory of objective function
            for(int i = 0 ; i < current_instance.dimension; i++){    
                for(int k = 0 ; k < current_instance.vehicles ; k++){       
                    
                    if(i != j)
                        sum +=  current_instance.c_distances[i][j] * x[i][j][k];
                }
            }
        }

        float w = 0.5;
        GRBLinExpr sum2 = 0;
        for(int i = 1; i < current_instance.dimension; i++){     //second somatory of objective function
            for(int k = 0 ; k < current_instance.vehicles ; k++){       
                sum2 += w * h[i][k];                  
            }
        }

        // Define a função objetivo
        GRBLinExpr obj = sum + sum2;
        modelo.setObjective(obj, GRB_MINIMIZE);

        // Adiciona as restrições
        
        //Extra constraint: need to limitate de H
        for(int i = 1 ; i < current_instance.dimension; i++){        //i E V' (vertices without 0)   
            for(int k = 0 ; k < current_instance.vehicles ; k++){       // k E K

                GRBLinExpr exp = 0;
                
                exp = h[i][k] - t[i][k] + current_instance.requests_[i]->due_date;
                
                sprintf(var, "c0_%d, %d", i, k);
                modelo.addConstr(exp >= 0, var);
            }
        }

        //Constraint 1: all requests must be served 
        for(int i = 1 ; i < current_instance.dimension; i++){        //i E V' (vertices without 0)
            GRBLinExpr sum = 0;
            for(int j = 0 ; j < current_instance.dimension; j++){    //j E V 
                for(int k = 0 ; k < current_instance.vehicles; k++){       // k E K
                    
                    if(i != j){
                        sum += x[i][j][k];
                    }
                }
            }

            sprintf(var, "c1_%d", i);
	    modelo.addConstr(sum == 1, var);
        }

        //Constraint 2: every vertex is served exactly once 
        for (int i = 1; i < current_instance.dimension; i++) {   
            for(int k = 0; k < current_instance.vehicles; k++){

                GRBLinExpr exp = 0;
                GRBLinExpr exp2 = 0;
                for (int j = 0; j < current_instance.dimension; j++){
                    
                    if(i != j){
                        exp += x[j][i][k];
                        exp2 += x[i][j][k];
                    }
                    
                }

                sprintf(var, "c2_%d,%d", i,k);
                modelo.addConstr(exp - exp2 == 0, var);
            }
        }  


        //Constraint 3  : first somatory - leaving depot
        for(int k = 0; k < current_instance.vehicles; k++){
            GRBLinExpr sum = 0;
            
            for (int j = 1; j < current_instance.dimension; j++){
                
                sum += x[0][j][k];
            }

            sprintf(var, "c3.I_%d", k);
            modelo.addConstr(sum == 1, var);
        }


        //Constraint 3  : second somatory - arriving depot
        for(int k = 0; k < current_instance.vehicles; k++){
            GRBLinExpr sum = 0;
            for (int j = 1; j < current_instance.dimension; j++){
                
                sum += x[j][0][k];
            }

            sprintf(var, "c3.II_%d", k);
            modelo.addConstr(sum == 1, var);
        }


        // Constraint 4 : to not overflow the capacity of the vehicle
        for(int k = 0; k < current_instance.vehicles; k++){        
            GRBLinExpr sum = 0;
            for(int j = 0 ; j < current_instance.dimension; j++){
                for(int i = 1 ; i < current_instance.dimension; i++){
                    
                    if(i != j)
                        sum += x[i][j][k] * current_instance.requests_[i]->demand;
                    
                }
            }

            sprintf(var, "c4_%d", k);
            modelo.addConstr(sum <= current_instance.capacity, var);
        }

        //IloNum M = 1e9;
        int M = 1e7;
        //Constraint 5                                          
        for(int i = 0; i < current_instance.dimension; i++){             
            for(int j = 1; j < current_instance.dimension; j++){
                for(int k = 0; k < current_instance.vehicles; k++){
                    
                    GRBLinExpr exp = 0;

                    if(i != j){
                        exp = t[i][k] + current_instance.c_distances[i][j] - M + (M *  x[i][j][k]);

                        sprintf(var, "c5_%d,%d,%d", i,j,k);
                        modelo.addConstr(exp - t[j][k] <= 0, var);
                    }        
                }
            }
        }

        //Constraint 6  - ensures that the vehicle leaves after the last release date
        for(int i = 1 ; i < current_instance.dimension; i++){           
            for(int k = 0; k < current_instance.vehicles; k++){

                GRBLinExpr sum = 0;
                for(int j = 0; j < current_instance.dimension; j++){

                    if(i != j)
                        sum += x[j][i][k];
                    
                }

                sum = sum * current_instance.requests_[i]->release_date;

                sprintf(var, "c6_%d,%d", i,k);
                modelo.addConstr(t[0][k] - sum >= 0, var);
            }
        }


        // Otimiza o modelo
        modelo.optimize();

        // Imprime a solução
        if (modelo.get(GRB_IntAttr_Status) == GRB_OPTIMAL) {
            std::cout << "Solução ótima encontrada!" << "valor:" << modelo.get(GRB_DoubleAttr_ObjVal) << std::endl;
            //std::cout << "Valor de x: " << x.get(GRB_DoubleAttr_X) << std::endl;
            //std::cout << "Valor de y: " << y.get(GRB_DoubleAttr_X) << std::endl;
        modelo.write("modelo.lp");
	
            for (int i = 0; i < current_instance.dimension; i++) {
                for (int j = 0; j < current_instance.dimension; j++) {
                    if(i != j){
                        for(int k = 0; k < current_instance.vehicles; k++){
            
                            if (x[i][j][k].get(GRB_DoubleAttr_X) > 0.5)
                                std::cout << "x_" << i << ", " << j << ", " <<  k <<" = " << x[i][j][k].get(GRB_DoubleAttr_X) << std::endl;
                        }
                    }
                }
            }
	    }

    }
    
    catch (GRBException& ex) {
        std::cout << "Erro no Gurobi: " << ex.getMessage() << std::endl;
    }
    catch (...) {
        std::cout << "Erro durante a execução." << std::endl;
    }

}

int main(int argc, char** argv) {

    if(argc < 2){
       printf("Correct command: ./bc data/\n");

       return 0;

    }else if(argc >= 2){
        
       Instance current_instance;
       string filename = string(argv[1]);

        current_instance.loadInstance(filename);           //passing the atributtes of the instance to the variables
       //current_instance.print();                          //printing the instances

        solve(current_instance);
    }
    

    return 0;
}
