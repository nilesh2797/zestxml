#pragma once

#include <iostream>
#include <chrono>
#include <time.h>
#include <sys/time.h>

#include "config.h"
#include "utils.h"

using namespace std;
using namespace std::chrono;


/* ----------------------------- timer resource ------------------------------------ */
#define SEC_PER_DAY   86400
#define SEC_PER_HOUR  3600
#define SEC_PER_MIN   60

inline void get_wallclock_time()
{
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	
	// Form the seconds of the day
	long hms = tv.tv_sec % SEC_PER_DAY;
	hms += tz.tz_dsttime * SEC_PER_HOUR;
	hms -= tz.tz_minuteswest * SEC_PER_MIN;
	// mod `hms` to insure in positive range of [0...SEC_PER_DAY)
	hms = (hms + SEC_PER_DAY) % SEC_PER_DAY;

	// Tear apart hms into h:m:s
	int hour = hms / SEC_PER_HOUR;
	int min = (hms % SEC_PER_HOUR) / SEC_PER_MIN;
	int sec = (hms % SEC_PER_HOUR) % SEC_PER_MIN; // or hms % SEC_PER_MIN

	// printf("Current local time: %d:%02d:%02d\n", hour, min, sec);
}

class Timer
{
	private:
		high_resolution_clock::time_point start_time;
		high_resolution_clock::time_point stop_time;

	public:
		Timer()
		{
		}

		void tic()
		{
			get_wallclock_time();
			start_time = high_resolution_clock::now();
		}

		float toc()
		{
			get_wallclock_time();
			stop_time = high_resolution_clock::now();
			float elapsed_time = duration_cast<microseconds>(stop_time - start_time).count() / 1000000.0;
			return elapsed_time;
		}

		float print_toc()
		{
			get_wallclock_time();
			stop_time = high_resolution_clock::now();
			float elapsed_time = duration_cast<microseconds>(stop_time - start_time).count() / 1000000.0;
			cout << "Elapsed time: " << elapsed_time << " s = " << elapsed_time/3600.0 << " hr" << endl;
			return elapsed_time;
		}
};

class TQDM
{
    Timer timer;
    int iter = 0;
    int total = 0;
    int step_size = 1;
    float speed = 0;
    float percentage = 0;
    float eta = 0;
    
public:
    TQDM(int _total = 0, int _step_size = 1)
    {
        init(_total, _step_size);
    }
    
    void init(int _total = 0, int _step_size = 1)
    {
        iter = 0;
        total = _total;
        step_size = _step_size;
        timer.tic();
    }
    
    void step()
    {
    	#pragma omp critical
    	{
	        iter++;
	        if(iter%step_size == 0) 
	        {
	        	speed 		= (float)iter / timer.toc();
	        	percentage 	= ((total > 0) ? iter*100.0/total : 0.0);
	        	eta 		= max((float)0.0, (total-iter)/speed);

	            LOG(fixed << iter << "/" << total << " (" << setprecision(1) << percentage 
	            	<< "%) iterations done at " << setprecision(2) << speed << " it/sec eta : " 
	            	<< setprecision(1) << eta << " s");
	        }

	        if(iter == total) finish();
   		}
    }

    void finish()
    {
    	speed 		= (float)iter / timer.toc();
    	percentage 	= ((total > 0.0) ? iter*100.0/total : 100.0);
    	LOGN(fixed << iter << "/" << total << " (" << setprecision(0) << percentage << "%) iterations done in " << setprecision(1) << timer.toc() << " seconds at " << setprecision(2) << speed << " it/sec");
    }
};