#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <sys/time.h>

int main()
{
    // Open the video camera.
    std::string pipeline = "libcamerasrc"
        " ! video/x-raw, width=800, height=600" // camera needs to capture at a higher resolution
        " ! videoconvert"
        " ! videoscale"
        " ! video/x-raw, width=400, height=300" // can downsample the image after capturing
        " ! videoflip method=rotate-180" // remove this line if the image is upside-down
        " ! appsink drop=true max_buffers=2";
    cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER);
    bool is_camera_open = cap.isOpened();
    if(!is_camera_open) {
        printf("Could not open camera.\n");
        return 1;
    }

    // Create the OpenCV window
    cv::namedWindow("Camera", cv::WINDOW_AUTOSIZE);
    cv::Mat frame_bgr, frame_hsv;

    // initialise variables to measure the frame rate - 
    int frame_id = 0;
    timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    // Create a control window
    cv::namedWindow("Control", cv::WINDOW_AUTOSIZE);
    int iLowH = 0;
    int iHighH = 179;

    int iLowS = 0;
    int iHighS = 255;

    int iLowV = 0;
    int iHighV = 255;

    int structuring_element_size = 5;
    int structuring_element_operation = 1;
    int morph_operation = cv::MORPH_OPEN;

    // Create trackbars in "Control" window for hsv thresholds
    cv::createTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
    cv::createTrackbar("HighH", "Control", &iHighH, 179);
    cv::createTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
    cv::createTrackbar("HighS", "Control", &iHighS, 255);
    cv::createTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
    cv::createTrackbar("HighV", "Control", &iHighV, 255);

    // create more control for for operation type and structuring element size
    cv::createTrackbar("structuring_element_size", "Control", &structuring_element_size, 10); //Value (0 - 10)
    cv::createTrackbar("structuring_element_operation", "Control", &structuring_element_operation, 1); //Value (0 - 1)


    // Create the display windows
    cv::namedWindow("Display", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("Thresholded", cv::WINDOW_AUTOSIZE);

    for(;;) {
        bool is_frame_read = cap.read(frame_bgr); // read frame from camera
        if (!is_frame_read) {
            printf("Could not read a frame.\n");
            break;
        }

        // Convert to HSV colour space
	    cv::cvtColor(frame_bgr, frame_hsv, cv::COLOR_BGR2HSV);

        //show frame
        cv::Mat thresh_img;

		// Threshold the image
		cv::inRange(frame_hsv, cv::Scalar(iLowH, iLowS, iLowV), cv::Scalar(iHighH, iHighS, iHighV), thresh_img);
        
        // Create the structuring element for performing cool stuff
        if(structuring_element_operation == 0){
            morph_operation = cv::MORPH_OPEN;
        } else {
            morph_operation = cv::MORPH_CLOSE;
        }
        // Clean up the image
        cv:: Mat structuring_element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(structuring_element_size + 1, structuring_element_size + 1));
        cv::morphologyEx(thresh_img, thresh_img, morph_operation, structuring_element);

        // Calculate the centroid
        cv::Moments m = cv::moments(thresh_img, true);
		bool is_nonzero_pixels_detected = m.m00 > 0;
        if(is_nonzero_pixels_detected){
            double x = m.m10 / m.m00;
            double y = m.m01 / m.m00;
            printf("Centroid at (%f, %f) \n", x, y);
        } else{
            printf("Centroid not detected (no nonzero pixels) \n");
        }

        // Show the thresholded and final images
		cv::imshow("Thresholded", thresh_img);
		cv::imshow("Display", frame_bgr);

		// Allow openCV to process GUI events
		cv::waitKey(100);

    }

    // Free the camera 
    cap.release();
    return 0;
}