#ifndef CLASSES_H_INCLUDED
#define CLASSES_H_INCLUDED

	#include <vector>
	#include <climits>
    #include <stdexcept>
    #include <fstream>

	using namespace std;

	/***********************************************************************************
     *  STRUCT: Request
     *********************/
	struct Request{

		int  no
			,release_date
            ,due_date
			,demand;
        
        double  x_pos
               ,y_pos;

		Request(){
			no = -1;
			x_pos = -1;
            y_pos = 0;
            release_date = 0;
            due_date = INT_MAX;
		}

		Request(int no, int x_pos, int y_pos, int demand, double release_date, double due_date){
			this->no = no;
			this->x_pos = x_pos;
            this->y_pos = y_pos;
			this->demand = demand;
            this->release_date = release_date;
            this->due_date = due_date;
		}
	};


#endif // CLASSES_H_INCLUDED