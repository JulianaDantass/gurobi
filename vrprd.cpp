#include <iostream>
#include <gurobi_c++.h>
#include <vector>

int main() {
    //teste na tora - que vai ser retirado
    int vertex = 20;
    int vehicles = 2;
    int capacity = 160;
    std::vector<int> due_date = {99999999, 77, 58, 68, 71, 63, 68, 68, 52, 52, 78, 56, 51, 63, 76, 68, 54, 60, 64, 78};
    std::vector <int> demand = {0, 19, 30, 16, 23, 11, 31, 15, 28, 8, 8, 7, 14, 6, 19, 11, 26, 17, 6, 15};
    std::vector <int> release_date = {0, 2, 3, 1, 5, 1, 1, 2, 1, 3, 2, 5, 5, 1, 4, 2, 3, 3, 5, 3};
    
    std::vector<std::vector<double>> c = 
    {{0.00, 14.00, 21.00, 33.00, 22.00, 23.00, 12.00, 22.00, 32.00, 32.00, 21.00, 28.00, 30.00, 29.00, 31.00, 30.00, 32.00, 39.00, 44.00, 16.00},
        {14.00, 0.00, 12.00, 19.00, 12.00, 24.00, 12.00, 19.00, 21.00, 27.00, 7.00, 19.00, 16.00, 21.00, 33.00, 17.00, 31.00, 27.00, 31.00, 19.00},
        {21.00, 12.00, 0.00, 15.00, 22.00, 16.00, 11.00, 9.00, 12.00, 15.00, 11.00, 29.00, 19.00, 9.00, 24.00, 23.00, 20.00, 19.00, 24.00, 15.00},
        {33.00, 19.00, 15.00, 0.00, 21.00, 31.00, 25.00, 23.00, 8.00, 24.00, 12.00, 25.00, 9.00, 17.00, 37.00, 16.00, 32.00, 10.00, 12.00, 30.00},
        {22.00, 12.00, 22.00, 21.00, 0.00, 36.00, 24.00, 30.00, 26.00, 37.00, 12.00, 7.00, 13.00, 30.00, 44.00, 9.00, 42.00, 31.00, 33.00, 30.00},
        {23.00, 24.00, 16.00, 31.00, 36.00, 0.00, 13.00, 8.00, 25.00, 13.00, 26.00, 43.00, 35.00, 16.00, 8.00, 39.00, 9.00, 32.00, 38.00, 7.00},
        {12.00, 12.00, 11.00, 25.00, 24.00, 13.00, 0.00, 10.00, 23.00, 20.00, 16.00, 31.00, 26.00, 17.00, 21.00, 28.00, 21.00, 30.00, 35.00, 7.00},
        {22.00, 19.00, 9.00, 23.00, 30.00, 8.00, 10.00, 0.00, 18.00, 10.00, 19.00, 37.00, 28.00, 9.00, 15.00, 32.00, 12.00, 24.00, 30.00, 9.00},
        {32.00, 21.00, 12.00, 8.00, 26.00, 25.00, 23.00, 18.00, 0.00, 17.00, 15.00, 32.00, 17.00, 10.00, 31.00, 23.00, 25.00, 7.00, 13.00, 26.00},
        {32.00, 27.00, 15.00, 24.00, 37.00, 13.00, 20.00, 10.00, 17.00, 0.00, 25.00, 44.00, 31.00, 7.00, 16.00, 37.00, 9.00, 21.00, 27.00, 18.00},
        {21.00, 7.00, 11.00, 12.00, 12.00, 26.00, 16.00, 19.00, 15.00, 25.00, 0.00, 19.00, 10.00, 18.00, 34.00, 13.00, 31.00, 21.00, 24.00, 22.00},
        {28.00, 19.00, 29.00, 25.00, 7.00, 43.00, 31.00, 37.00, 32.00, 44.00, 19.00, 0.00, 16.00, 37.00, 51.00, 10.00, 49.00, 35.00, 36.00, 38.00},
        {30.00, 16.00, 19.00, 9.00, 13.00, 35.00, 26.00, 28.00, 17.00, 31.00, 10.00, 16.00, 0.00, 24.00, 43.00, 6.00, 38.00, 19.00, 20.00, 32.00},
        {29.00, 21.00, 9.00, 17.00, 30.00, 16.00, 17.00, 9.00, 10.00, 7.00, 18.00, 37.00, 24.00, 0.00, 21.00, 30.00, 15.00, 16.00, 22.00, 18.00},
        {31.00, 33.00, 24.00, 37.00, 44.00, 8.00, 21.00, 15.00, 31.00, 16.00, 34.00, 51.00, 43.00, 21.00, 0.00, 47.00, 7.00, 36.00, 42.00, 15.00},
        {30.00, 17.00, 23.00, 16.00, 9.00, 39.00, 28.00, 32.00, 23.00, 37.00, 13.00, 10.00, 6.00, 30.00, 47.00, 0.00, 43.00, 26.00, 26.00, 35.00},
        {32.00, 31.00, 20.00, 32.00, 42.00, 9.00, 21.00, 12.00, 25.00, 9.00, 31.00, 49.00, 38.00, 15.00, 7.00, 43.00, 0.00, 30.00, 36.00, 16.00}, 
        {39.00, 27.00, 19.00, 10.00, 31.00, 32.00, 30.00, 24.00, 7.00, 21.00, 21.00, 35.00, 19.00, 16.00, 36.00, 26.00, 30.00, 0.00, 6.00, 33.00},
        {44.00, 31.00, 24.00, 12.00, 33.00, 38.00, 35.00, 30.00, 13.00, 27.00, 24.00, 36.00, 20.00, 22.00, 42.00, 26.00, 36.00, 6.00, 0.00, 38.00},
        {16.00, 19.00, 15.00, 30.00, 30.00, 7.00, 7.00, 9.00, 26.00, 18.00, 22.00, 38.00, 32.00, 18.00, 15.00, 35.00, 16.00, 33.00, 38.00, 0.00}
        };

    
    try {
        // Cria o ambiente Gurobi
        GRBEnv env = GRBEnv();
        env.set(GRB_IntParam_OutputFlag, 0); // Desativa a saída no console do Gurobi

        // Cria o modelo
        GRBModel modelo = GRBModel(env);

        // Cria as variáveis de decisão
        char var[100];                        
    
        GRBVar*** x = new GRBVar**[vertex];

        for (int i = 0; i < vertex; i++) {
            x[i] = new GRBVar*[vertex];
            for (int j = 0; j < vertex; j++) {
                x[i][j] = new GRBVar[vehicles];
                for (int k = 0; k < vehicles; k++) {

                    if (i != j){
                        sprintf(var, "x(%d,%d,%d)", i, j, k);
                        x[i][j][k] = modelo.addVar(0.0, 1.0, 0.0, GRB_BINARY, var);
                    }
                }
            }
        }

        GRBVar** t = new GRBVar*[vertex];

        for (int i = 0; i < vertex; i++) {
            t[i] = new GRBVar[vehicles];
            for (int j = 0; j < vehicles; j++) {

                    sprintf(var, "t(%d,%d)", i, j);
                    t[i][j] =  modelo.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, var);
            }
        }

        GRBVar** h = new GRBVar*[vertex];

        for (int i = 1; i < vertex; i++) {
            h[i] = new GRBVar[vehicles];
            for (int j = 0; j < vehicles; j++) {

                    sprintf(var, "h(%d,%d)", i, j);
                    h[i][j] =  modelo.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, var);
                }
                
            
        }
    
        GRBLinExpr sum = 0;
        for(int j = 0 ; j < vertex; j++){        //first somatory of objective function
            for(int i = 0 ; i < vertex; i++){    
                for(int k = 0 ; k < vehicles ; k++){       
                    
                    if(i != j)
                        sum +=  c[i][j] * x[i][j][k];
                }
            }
        }

        float w = 0.5;
        GRBLinExpr sum2 = 0;
        for(int i = 1; i < vertex; i++){     //second somatory of objective function
            for(int k = 0 ; k < vehicles ; k++){       
                sum2 += w * h[i][k];                  
            }
        }

        // Define a função objetivo
        GRBLinExpr obj = sum + sum2;
        modelo.setObjective(obj, GRB_MINIMIZE);

        // Adiciona as restrições
        
        //Extra constraint: need to limitate de H
        for(int i = 1 ; i < vertex; i++){        //i E V' (vertices without 0)   
            for(int k = 0 ; k < vehicles ; k++){       // k E K

                GRBLinExpr exp = 0;
                
                exp = h[i][k] - t[i][k] + due_date[i];
                
                sprintf(var, "c0_%d, %d", i, k);
                modelo.addConstr(exp >= 0, var);
            }
        }

        //Constraint 1: all requests must be served 
        for(int i = 1 ; i < vertex; i++){        //i E V' (vertices without 0)
            GRBLinExpr sum = 0;
            for(int j = 0 ; j < vertex; j++){    //j E V 
                for(int k = 0 ; k < vehicles; k++){       // k E K
                    
                    if(i != j){
                        sum += x[i][j][k];
                    }
                }
            }

            sprintf(var, "c1_%d", i);
	    modelo.addConstr(sum == 1, var);
        }

        //Constraint 2: every vertex is served exactly once 
        for (int i = 1; i < vertex; i++) {   
            for(int k = 0; k < vehicles; k++){

                GRBLinExpr exp = 0;
                GRBLinExpr exp2 = 0;
                for (int j = 0; j < vertex; j++){
                    
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
        for(int k = 0; k < vehicles; k++){
            GRBLinExpr sum = 0;
            
            for (int j = 1; j < vertex; j++){
                
                sum += x[0][j][k];
            }

            sprintf(var, "c3.I_%d", k);
            modelo.addConstr(sum == 1, var);
        }


        //Constraint 3  : second somatory - arriving depot
        for(int k = 0; k < vehicles; k++){
            GRBLinExpr sum = 0;
            for (int j = 1; j < vertex; j++){
                
                sum += x[j][0][k];
            }

            sprintf(var, "c3.II_%d", k);
            modelo.addConstr(sum == 1, var);
        }


        // Constraint 4 : to not overflow the capacity of the vehicle
        for(int k = 0; k < vehicles; k++){        
            GRBLinExpr sum = 0;
            for(int j = 0 ; j < vertex; j++){
                for(int i = 1 ; i < vertex; i++){
                    
                    if(i != j)
                        sum += x[i][j][k] * demand[i];
                    
                }
            }

            sprintf(var, "c4_%d", k);
            modelo.addConstr(sum <= capacity, var);
        }

        //IloNum M = 1e9;
        int M = 1e8;
        //Constraint 5                                          
        for(int i = 0; i < vertex; i++){             
            for(int j = 1; j < vertex; j++){
                for(int k = 0; k < vehicles; k++){
                    
                    GRBLinExpr exp = 0;

                    if(i != j){
                        exp = t[i][k] + c[i][j] - M + (M *  x[i][j][k]);

                        sprintf(var, "c5_%d,%d,%d", i,j,k);
                        modelo.addConstr(exp - t[j][k] <= 0, var);
                    }        
                }
            }
        }

        //Constraint 6  - ensures that the vehicle leaves after the last release date
        for(int i = 1 ; i < vertex; i++){           
            for(int k = 0; k < vehicles; k++){

                GRBLinExpr sum = 0;
                for(int j = 0; j < vertex; j++){

                    if(i != j)
                        sum += x[j][i][k];
                    
                }

                sum = sum * release_date[i];

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
	
	    for (int i = 0; i < vertex; i++) {
            for (int j = 0; j < vertex; j++) {
			    if(i != j){
				    for(int k = 0; k < vehicles; k++){
		
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

    return 0;
}
