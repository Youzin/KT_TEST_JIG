
#include <stdio.h>
#include	"util.h"
#include 	"rabm.h"


extern OP_STATUS		op_sts;
int pi_control_flag = 0;

#define KP	0.2
#define KI  0.15
#define MAX_PI_N	100
#define	MAX_MA		10

int	err_ind = 0, ma_ind = 0, err_flag, ma_flag;
float ma[MAX_MA];

double err[MAX_PI_N], err_sum, err_avg, err_old, ma_sum;
double Kp, Ki;
double error;
int current_limit_flag;

#define CL_OFFSET 1.0


void init_pi_limit()
{
	err_ind = 0;
	ma_ind = 0;
	ma_sum =0.0;
	err_sum = 0.0;
	err_flag = 0;
	ma_flag = 0;
}


void pi_current_limit()
{	
	//static double offset = 0.95;
	//int sign;
	double ref;
	static int out_ok = 1, count = 0;




	rect_sts.bat_A = read_bat_A();
        if ( pi_control_flag == 0 || op_sts.current_mode == 0 ) return;
	error = (double) (op_sts.charge_A * CL_OFFSET  - rect_sts.bat_A);

	if ( out_ok && error > 0.2 ) {			
		current_limit_flag = 0;
		err_ind = 0;
		ma_sum = err_sum = 0.0;
		ma_ind = 0;
		ma_flag = 0;
		err_flag = 0;
		return;
	}

	current_limit_flag = 1;

	if ( ma_flag == 0 ) {
		ma[ma_ind] = error;
		ma_sum += error;
		err_avg= ma_sum / (ma_ind+1);
	}
	else {
		ma_sum -= ma[ma_ind];
		ma[ma_ind] = error;
		ma_sum += error;
		err_avg= ma_sum / MAX_MA;
	}
	ma_ind++;
	if (ma_ind >= MAX_MA ) {
		ma_flag = 1;	
		ma_ind = 0;
	}	

	
	if ( err_flag == 0 ) {		
		err[err_ind] = error;
		err_sum += error;		
	}
	else {
		err_sum -= err[err_ind];
		err[err_ind] = error;
		err_sum += error;
	}	
	
	err_ind++;
	if (  err_ind >= MAX_PI_N ) {
		err_flag = 1;
		err_ind = 0;
	}

	 if ( !ma_flag || !err_flag ) return;
	


	ref =  (err_avg * KP + err_sum * KI) + op_sts.output_V;
	//printf("C%5.2fV %5.2f\n", ref, rect_sts.bat_A);
	if ( ref >= op_sts.output_V ) {
		ref = op_sts.output_V;	
		out_ok = 1;
	}
	else {
		if ( ref < MIN_OUT_V ) ref = MIN_OUT_V;
		out_ok = 0;
	}
	
	write_output_voltage((float) ref);

	if ( count++ > 100 ) {
		printf("C%5.2fV\n", rect_sts.bat_A);
		count = 0;
	}

}





