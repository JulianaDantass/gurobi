#include <gurobi_c++.h>
#include <iostream>
#include "instance.h"
#include <vector>


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
        int M = 1e8;
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
       current_instance.print();                          //printing the instances

        solve(current_instance);
    }

    
    
    

    return 0;
}
