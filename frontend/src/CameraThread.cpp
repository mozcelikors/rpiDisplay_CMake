#include "CameraThread.h"
#include <QObject>
#include "QDebug"

#include <ctime>
#include <iostream>
#include <unistd.h>

#include "common.h"

#include "camera_interface.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core.hpp>
#include <raspicam/raspicam_cv.h>

#include "main.h"

using namespace std;
using namespace cv;

void CameraThread::run()
{
	raspicam::RaspiCam_Cv Camera;

	/* 240x320 since we'll rotate, 320x240 if we didn't rotate */
	Camera.set(CV_CAP_PROP_FRAME_WIDTH, APP_HEIGHT);
	Camera.set(CV_CAP_PROP_FRAME_HEIGHT,APP_WIDTH);
	Camera.set(CAP_PROP_BRIGHTNESS, 70);
	Camera.set(CAP_PROP_CONTRAST, 70);

	cv::Mat image;

	//set camera params
	Camera.set( CV_CAP_PROP_FORMAT, CV_8UC3 ); // CV_8UC1 -> one color

	//Create directory if does not exist.
	system("mkdir -p /home/root/projects/camera/");

	//Open camera
	if (!Camera.open()) {cerr<<"Error opening the camera"<<endl;}

	while (1)
	{
		if (csi.camera_active_f == 1) /* Camera active */
		{
			if (!Camera.isOpened())
				Camera.open();

			Camera.retrieve ( image);
			Camera.grab();

			/* Rotate */
			double angle = -90;
			// get rotation matrix for rotating the image around its center in pixel coordinates
			cv::Point2f center((image.cols-1)/2.0, (image.rows-1)/2.0);
			cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
			// determine bounding rectangle, center not relevant
			cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), image.size(), angle).boundingRect2f();
			// adjust transformation matrix
			rot.at<double>(0,2) += bbox.width/2.0 - image.cols/2.0;
			rot.at<double>(1,2) += bbox.height/2.0 - image.rows/2.0;
			cv::Mat dst;
			cv::warpAffine(image, dst, rot, bbox.size());

			sem_wait(&csi.camera_sem);
#ifdef CAMERA_DISPLAY_METHOD_1
			csi.camera_image = dst;
#else
#ifdef CAMERA_DISPLAY_METHOD_2
			cv::imwrite("/home/root/projects/camera/embeddev_cv_image.jpg", dst);
#endif
#endif
			sem_post(&csi.camera_sem);
		}
		else /* Camera non active */
		{
			if (Camera.isOpened())
				Camera.release();
		}

		usleep(10);

	}

	Camera.release();
}


