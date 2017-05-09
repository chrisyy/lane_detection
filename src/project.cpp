#include <cv.h>
#include <highgui.h>
#include <stdio.h>


#define WIN_WIDTH	10
#define WIN_HEIGHT	WIN_WIDTH

#define SCAN_WIDTH	20
#define SCAN_HEIGHT	20

#define CAL_WIDTH	5
#define CAL_HEIGHT	CAL_WIDTH

#define JUMP_LINE	3
#define SCAN_TIMES	3

#define SPEED		0

#define MIN_NUM		5

#define JITTER		8

#define ADJUST		10

//#define JUMP_FRAME

//#define LEFT_LANE


#ifdef LEFT_LANE
CvPoint LeftDetect(IplImage *processed, int x, int y){
	int x_pos = x, y_pos = y;
	bool found = false;
	int times = 0;
	while(1){
		if(found)
			return cvPoint(x_pos, y_pos + SCAN_HEIGHT);
		else
			cvSetImageROI(processed, cvRect(x_pos, y_pos, WIN_WIDTH, WIN_HEIGHT));

		cvSmooth(processed, processed, CV_GAUSSIAN, 3, 3);
		cvCanny(processed, processed, 50, 100);
		cvResetImageROI(processed);

		int count = 0;
		for(int j = y_pos; j < y_pos + WIN_HEIGHT; j++){
			for(int i = x_pos; i < x_pos + WIN_WIDTH; i++){
				if(*cvPtr2D(processed, j, i) <= 10) continue;
				else{
					count++;
					if(count >= MIN_NUM){
						j = y_pos + WIN_HEIGHT;
						break;
					}
				}
			}
		}

		if(count >= MIN_NUM){
			x_pos += WIN_WIDTH;
			y_pos -= SCAN_HEIGHT;
			found = true;
		}

		if(!found){
			if(x_pos - WIN_WIDTH >= 0) x_pos -= WIN_WIDTH;
			else y_pos -= WIN_HEIGHT;
			if(y_pos < processed->height / 2){
				if(times >= SCAN_TIMES) break;
				else times++;
				x_pos = x;
				y_pos = y - times * JUMP_LINE * WIN_HEIGHT;
			}
		}
	}

	return cvPoint(0, 0);
}
#endif

/* return distance structure, (0, 0) means failure */
CvPoint RightDetect(IplImage *processed, int x, int y){
	int x_pos = x, y_pos = y;
	bool found = false;
	int times = 0;
	while(1){
		if(found)
			return cvPoint(x_pos + SCAN_WIDTH, y_pos + SCAN_HEIGHT);
		else
			cvSetImageROI(processed, cvRect(x_pos, y_pos, WIN_WIDTH, WIN_HEIGHT));

		cvSmooth(processed, processed, CV_GAUSSIAN, 3, 3);
		cvCanny(processed, processed, 50, 100);
		cvResetImageROI(processed);

		int count = 0;
		for(int j = y_pos; j < y_pos + WIN_HEIGHT; j++){
			for(int i = x_pos; i < x_pos + WIN_WIDTH; i++){
				if(*cvPtr2D(processed, j, i) <= 10) continue;
				else{
					count++;
					if(count >= MIN_NUM){
						j = y_pos + WIN_HEIGHT;
						break;
					}
				}
			}
		}

		if(count >= MIN_NUM){
			x_pos -= SCAN_WIDTH;
			y_pos -= SCAN_HEIGHT;
			found = true;
		}

		if(!found){
			if(x_pos + WIN_WIDTH + WIN_WIDTH <= processed->width) x_pos += WIN_WIDTH;
			else y_pos -= WIN_HEIGHT;
			if(y_pos < processed->height / 2){
				if(times >= SCAN_TIMES) break;
				else times++;
				x_pos = x;
				y_pos = y - times * JUMP_LINE * WIN_HEIGHT;
			}
		}
	}

	return cvPoint(0, 0);
}


CvPoint Right[10];
#ifdef LEFT_LANE
CvPoint Left[10];
#endif

void Calibrate(IplImage *processed)
{
	int x_pos, y_pos;
	int counter;
	int temp1, temp2, temp3;
	temp1 = processed->width / 2;
	temp2 = (processed->height / 4) * 3;
	temp3 = (((processed->height / 4) * 3) - (processed->height / 2)) / 10;

	/* right calibration */
	counter = 0;
	while(counter != 10){
		x_pos = temp1;
		y_pos = temp2 - temp3 * counter;

		while(true){
			cvSetImageROI(processed, cvRect(x_pos, y_pos, CAL_WIDTH, CAL_HEIGHT));
			cvSmooth(processed, processed, CV_GAUSSIAN, 3, 3);
			cvCanny(processed, processed, 50, 80);
			cvResetImageROI(processed);

			int total = 0;
			for(int j = y_pos; j < y_pos + CAL_HEIGHT; j++){
				for(int i = x_pos; i < x_pos + CAL_WIDTH; i++){
					if(*cvPtr2D(processed, j, i) <= 10) continue;
					else{
						total++;
						if(total >= MIN_NUM){
							j = y_pos + CAL_HEIGHT;
							break;
						}
					}
				}
			}

			if(total >= MIN_NUM){
				Right[counter].x = x_pos;
				Right[counter].y = y_pos;
				break;
			}
			else{
				if(x_pos + CAL_WIDTH + CAL_WIDTH <= processed->width) x_pos += CAL_WIDTH;
				else{
					Right[counter].x = processed->width;
					Right[counter].y = y_pos;
					break;
				}
			}
		}//while

		counter++;
	}

#ifdef LEFT_LANE
	/* left calibration */
	counter = 0;
	while(counter != 10){
		x_pos = temp1;
		y_pos = temp2 - temp3 * counter;

		while(true){
			cvSetImageROI(processed, cvRect(x_pos, y_pos, CAL_WIDTH, CAL_HEIGHT));
			cvSmooth(processed, processed, CV_GAUSSIAN, 3, 3);
			cvCanny(processed, processed, 50, 100);
			cvResetImageROI(processed);

			int total = 0;
			for(int j = y_pos; j < y_pos + CAL_HEIGHT; j++){
				for(int i = x_pos; i < x_pos + CAL_WIDTH; i++){
					if(*cvPtr2D(processed, j, i) <= 10) continue;
					else{
						total++;
						if(total >= MIN_NUM){
							j = y_pos + CAL_HEIGHT;
							break;
						}
					}
				}
			}

			if(total >= MIN_NUM){
				Left[counter].x = x_pos + CAL_WIDTH;
				Left[counter].y = y_pos;
				break;
			}
			else{
				if(x_pos - CAL_WIDTH >= 0) x_pos -= CAL_WIDTH;
				else{
					Left[counter].x = 0;
					Left[counter].y = y_pos;
					break;
				}
			}
		}//while

		counter++;
	}
#endif
}

int main()
{
	const char* imgname= "track.avi";
	CvCapture *pCapture = cvCreateFileCapture(imgname);
	IplImage *img;
	cvNamedWindow("control");
	cvNamedWindow("processed");
	IplImage *processed;
	IplImage *l_img, *r_img, *s_img, *p_img;
	bool first = true;

	l_img = cvLoadImage("left.jpg");
	r_img = cvLoadImage("right.jpg");
	s_img = cvLoadImage("straight.jpg");
	p_img = cvLoadImage("stop.jpg");

	__int64 Frequency;
	__int64 start, stop;
	QueryPerformanceFrequency((LARGE_INTEGER *)&Frequency);

	while(img = cvQueryFrame(pCapture)){
		QueryPerformanceCounter((LARGE_INTEGER *)&start);

		//turn the image into a binary one
		processed = cvCreateImage(cvSize(img->width, img->height), img->depth, 1);
		cvCvtColor(img, processed, CV_RGB2GRAY);
		
		if(first){
			first = false;
			Calibrate(processed);
		}
		else{
			int direction = 0;		/* -1 for left, 0 for straight, 1 for right, -2 for failure */
			int displacement;
			CvPoint right = RightDetect(processed, processed->width / 2, 
				(processed->height / 4) * 3 - SPEED * JUMP_LINE * WIN_HEIGHT);

			if(right.x == 0 && right.y == 0){	/* fail to detect */
				direction = -2;
#ifndef LEFT_LANE
				/* slow down */
				cvShowImage("control", p_img);
#endif
			}
			else{
				int min = 10000;
				int index = 0;
				for(int i = 0; i < 10; i++){
					if(right.y > Right[i].y && (right.y - Right[i].y < min)){
						min = right.y - Right[i].y;
						index = i;
					}
					else if(Right[i].y > right.y && (Right[i].y - right.y) < min){
						min = Right[i].y - right.y;
						index = i;
					}
				}

				if(right.x > (Right[index].x - ADJUST) && (right.x - (Right[index].x - ADJUST) > JITTER)){
					direction = 1;
					displacement = right.x - (Right[index].x - ADJUST);
#ifndef LEFT_LANE
					/* turn right */
					cvShowImage("control", r_img);
#endif
				}
				else if((Right[index].x - ADJUST) > right.x && ((Right[index].x - ADJUST) - right.x > JITTER)){
					direction = -1;
					displacement = (Right[index].x - ADJUST) - right.x;
#ifndef LEFT_LANE
					/* turn left */
					cvShowImage("control", l_img);
#endif
				}
				else{
					direction = 0;
#ifndef LEFT_LANE
					/* go straight */
					cvShowImage("control", s_img);
#endif
				}
			}

#ifdef LEFT_LANE
			CvPoint left = LeftDetect(processed, processed->width / 2, 
				(processed->height / 4) * 3 - SPEED * JUMP_LINE * WIN_HEIGHT);

			if(left.x == 0 && left.y == 0){		/* fail to detect */
				switch(direction){
					case -2: cvShowImage("control", p_img); break;	/* slow down */
					case -1: cvShowImage("control", l_img); break;	/* turn left */
					case 0: cvShowImage("control", s_img); break;	/* go straight */
					case 1: cvShowImage("control", r_img); break;	/* turn right */
				}
			}
			else{
				int min2 = 10000;
				int index2 = 0;
				for(int i = 0; i < 10; i++){
					if(left.y > Left[i].y && (left.y - Left[i].y < min2)){
						min2 = left.y - Left[i].y;
						index2 = i;
					}
					else if(Left[i].y > left.y && (Left[i].y - left.y) < min2){
						min2 = Left[i].y - left.y;
						index2 = i;
					}
				}

				if(left.x > Left[index2].x && (left.x - Left[index2].x > JITTER)){
					/* right, combine left and right information to make decision */
					if(direction == -1){
						if(displacement >= left.x - Left[index2].x){
							/* turn left */
							cvShowImage("control", l_img);
						}
						else{
							/* turn right */
							cvShowImage("control", r_img);
						}
					}
				}
				else if(Left[index2].x > left.x && (Left[index2].x - left.x > JITTER)){
					/* left, combine left and right information to make decision */
					if(direction == 1){
						if(displacement >= Left[index2].x - left.x){
							/* turn right */
							cvShowImage("control", r_img);
						}
						else{
							/* turn left */
							cvShowImage("control", l_img);
						}
					}
				}
				else{
					/* straight, combine left and right information to make decision */
					switch(direction){
						case -2: cvShowImage("control", s_img); break;	/* go straight */
						case -1: cvShowImage("control", l_img); break;	/* turn left */
						case 0: cvShowImage("control", s_img); break;	/* go straight */
						case 1: cvShowImage("control", r_img); break;	/* turn right */
					}
				}
			}
#endif

		}

		cvShowImage("processed",processed);

#ifdef JUMP_FRAME
		for(int a = 0; a < SPEED; a++)
			cvGrabFrame(pCapture);
#endif
		
		QueryPerformanceCounter((LARGE_INTEGER *)&stop);
		printf("Processing time: %f\n", (double)(stop - start) / (double)Frequency);

		cvWaitKey(1);
	}

	cvWaitKey();
	cvReleaseCapture(&pCapture);
	cvReleaseImage(&processed);
	cvDestroyWindow("control");
	cvDestroyWindow("processed");
	cvReleaseImage(&r_img);
	cvReleaseImage(&l_img);
	cvReleaseImage(&s_img);
	cvReleaseImage(&p_img);
	return 0;
}