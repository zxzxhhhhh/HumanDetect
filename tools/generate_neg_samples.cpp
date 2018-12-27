#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
using namespace cv;
using namespace std;


int main()
{
	Mat image;
	image = imread("C:\\Users\\zxu07\\Desktop\\crop.png", -1);//改成自己的图片路径
	int width = 64, height = 128;  //切图的大小范围是30*30-100*100
	int number = 10;  //切400 个图
	for (int i = 0; i < number; i++)
	{
		int x = rand() % (image.cols - width), y = rand() % (image.rows - height);
		Mat sub_src;
		Rect rect(x, y, width, height);
		image(rect).copyTo(sub_src);
		string tempDir = "C:\\Users\\zxu07\\Desktop\\";   //切好后的图的储存路径
		stringstream ss;
		ss << i;
		string str = ss.str();// 给图像命名为 1.jpg 2.jpg ……
		tempDir += str;
		tempDir += ".jpg";
		imwrite(tempDir, sub_src);
	}

	return 0;

}
