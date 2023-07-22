#ifndef INSTANCE_H_INCLUDED
#define INSTANCE_H_INCLUDED

    #include <iomanip>
    #include <fstream>
    #include <vector>
    #include "classes.h"

    using namespace std;


    /***********************************************************************************
     *  CLASS: Instance
     *
     *  @brief Read an instance file and store its data
     *
     *  @version 1.0.0
     *  @date August 22, 2022
     ***********************************************************************************/
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

        private: // Private methods
            void init();
            void destroy();
            void clear();
            
            

        public: // Public methods
            Instance();
            ~Instance();

            double CalcDistEucledian(double &x_1, double &x_2, double &y_1, double &y_2);
            bool loadInstance(string instance_path);
            void print();
            vector<string> treating_string(string to_edit);
    };


#endif // INSTANCE_H_INCLUDED