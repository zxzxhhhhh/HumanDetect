#include "opencv2/opencv.hpp"
#include "opencv2/ml.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
#include <cctype>
#include <windows.h>
#include <vector>

using namespace std;

#define FILEPATH  "C:/Users/zxu07/Desktop/hog_pedestran_detect_c_plus_plus-master/hog_pedestran_detect_c_plus_plus-master/Pedestran_Detect/Pedestrians64x128/"

void Train()
{
	////////////////////////////////����ѵ������ͼƬ·�������///////////////////////////////////////////////////
	//ͼ��·�������
	vector<string> imagePath;
	vector<int> imageClass;
	string buffer;
	ifstream trainingData(string(FILEPATH) + "TrainData.txt");
	int numOfLine = 0;
	while (!trainingData.eof())
	{
		getline(trainingData, buffer);
		//cout << buffer << endl;
		if (!buffer.empty())
		{
			numOfLine++;
			if (numOfLine % 2 == 0)
			{
				//��ȡ�������
				imageClass.push_back(atoi(buffer.c_str()));
			}
			else
			{
				//��ȡͼ��·��
				imagePath.push_back(buffer);
			}
		}
	}
	trainingData.close();

	////////////////////////////////��ȡ������HOG����///////////////////////////////////////////////////
	//����������������
	int numOfSample = numOfLine / 2;
	cv::Mat featureVectorOfSample(numOfSample, 3780, CV_32FC1);

	//���������
	cv::Mat classOfSample(numOfSample, 1, CV_32SC1);

	cv::Mat convertedImg;
	cv::Mat trainImg;

	for (vector<string>::size_type i = 0; i < imagePath.size(); i++)
	{
		cout << "Processing: " << imagePath[i] << endl;
		cv::Mat src = cv::imread(imagePath[i], -1);
		if (src.empty())
		{
			cout << "can not load the image:" << imagePath[i] << endl;
			continue;
		}

		cv::resize(src, trainImg, cv::Size(64, 128));

		//��ȡHOG���� HOGDescriptor (Size _winSize, Size _blockSize, Size _blockStride, Size _cellSize, int _nbins��
		cv::HOGDescriptor hog(cv::Size(64, 128), cv::Size(16, 16), cv::Size(8, 8), cv::Size(8, 8), 9);
		vector<float> descriptors;
		hog.compute(trainImg, descriptors);

		cout << "hog feature vector: " << descriptors.size() << endl;

		for (vector<float>::size_type j = 0; j < descriptors.size(); j++)
		{
			featureVectorOfSample.at<float>(i, j) = descriptors[j];
		}
		classOfSample.at<int>(i, 0) = imageClass[i];
	}

	cout << "size of featureVectorOfSample: " << featureVectorOfSample.size() << endl;
	cout << "size of classOfSample: " << classOfSample.size() << endl;

	///////////////////////////////////ʹ��SVM������ѵ��///////////////////////////////////////////////////
	//���ò�����ע��Ptr��ʹ��
	cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::create();
	svm->setType(cv::ml::SVM::C_SVC);
	svm->setKernel(cv::ml::SVM::LINEAR);
	svm->setTermCriteria(cv::TermCriteria(1, 1000, FLT_EPSILON));

	//ѵ��SVM
	svm->train(featureVectorOfSample, cv::ml::ROW_SAMPLE, classOfSample);

	//����ѵ���õķ����������а�����SVM�Ĳ�����֧������������rho��
	svm->save(string(FILEPATH) + "classifier.xml");

	/*
	SVMѵ����ɺ�õ���XML�ļ����棬��һ�����飬����support vector������һ�����飬����alpha,��һ��������������rho;
	��alpha����ͬsupport vector��ˣ�ע�⣬alpha*supportVector,���õ�һ������������������ǰ�����-1��֮���ٸ���������������һ��Ԫ��rho��
	��ˣ���õ���һ�������������ø÷�������ֱ���滻opencv�����˼��Ĭ�ϵ��Ǹ���������cv::HOGDescriptor::setSVMDetector()����
	*/
	//��ȡ֧������
	cv::Mat supportVector = svm->getSupportVectors();

	//��ȡalpha��rho
	cv::Mat alpha;
	cv::Mat svIndex;
	float rho = svm->getDecisionFunction(0, alpha, svIndex);

	//ת������:����һ��Ҫע�⣬��Ҫת��Ϊ32��
	cv::Mat alpha2;
	alpha.convertTo(alpha2, CV_32FC1);

	//������������������
	cv::Mat result(1, 3780, CV_32FC1);
	result = alpha2 * supportVector;

	//����-1������Ϊʲô�����-1��
	//ע����Ϊsvm.predictʹ�õ���alpha*sv*another-rho�����Ϊ���Ļ�����Ϊ������������HOG�ļ�⺯���У�ʹ��rho+alpha*sv*another(anotherΪ-1)
	for (int i = 0; i < 3780; i++)
		result.at<float>(0, i) *= -1;

	//�����������浽�ļ�������HOGʶ��
	//��������������б����Ĳ���(��)��HOG����ֱ��ʹ�øò�������ʶ��
	FILE *fp = fopen((string(FILEPATH) + "HOG_SVM.txt").c_str(), "wb");
	for (int i = 0; i < 3780; i++)
	{
		fprintf(fp, "%f \n", result.at<float>(0, i));
	}
	fprintf(fp, "%f", rho);

	fclose(fp);
}

void Detect()
{
	cv::Mat img;
	FILE* f = 0;
	char _filename[1024];

	// ��ȡ����ͼƬ�ļ�·��
	f = fopen((string(FILEPATH) + "TestData.txt").c_str(), "rt");
	if (!f)
	{
		fprintf(stderr, "ERROR: the specified file could not be loaded\n");
		return;
	}

	//����ѵ���õ��б����Ĳ���(ע�⣬��svm->save����ķ�������ͬ)
	vector<float> detector;
	ifstream fileIn(string(FILEPATH) + "HOG_SVM.txt", ios::in);
	float val = 0.0f;
	while (!fileIn.eof())
	{
		fileIn >> val;
		detector.push_back(val);
	}
	fileIn.close();

	//����HOG
	cv::HOGDescriptor hog;
	hog.setSVMDetector(detector);
	//hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
	cv::namedWindow("people detector", 1);

	// ���ͼƬ
	for (;;)
	{
		// ��ȡ�ļ���
		char* filename = _filename;
		if (f)
		{
			if (!fgets(filename, (int)sizeof(_filename) - 2, f))
				break;
			if (filename[0] == '#')
				continue;

			//ȥ���ո�
			int l = (int)strlen(filename);
			while (l > 0 && isspace(filename[l - 1]))
				--l;
			filename[l] = '\0';
			img = cv::imread(filename);
		}
		printf("%s:\n", filename);
		if (!img.data)
			continue;

		fflush(stdout);
		vector<cv::Rect> found, found_filtered;
		// run the detector with default parameters. to get a higher hit-rate
		// (and more false alarms, respectively), decrease the hitThreshold and
		// groupThreshold (set groupThreshold to 0 to turn off the grouping completely).
		//��߶ȼ��
		hog.detectMultiScale(img, found, 0, cv::Size(8, 8), cv::Size(32, 32), 1.05, 2);

		size_t i, j;
		//ȥ���ռ��о������������ϵ�����򣬱������
		for (i = 0; i < found.size(); i++)
		{
			cv::Rect r = found[i];
			for (j = 0; j < found.size(); j++)
				if (j != i && (r & found[j]) == r)
					break;
			if (j == found.size())
				found_filtered.push_back(r);
		}

		// �ʵ���С����
		for (i = 0; i < found_filtered.size(); i++)
		{
			cv::Rect r = found_filtered[i];
			// the HOG detector returns slightly larger rectangles than the real objects.
			// so we slightly shrink the rectangles to get a nicer output.
			r.x += cvRound(r.width*0.1);
			r.width = cvRound(r.width*0.8);
			r.y += cvRound(r.height*0.07);
			r.height = cvRound(r.height*0.8);
			rectangle(img, r.tl(), r.br(), cv::Scalar(0, 255, 0), 3);
		}

		imshow("people detector", img);
		int c = cv::waitKey(0) & 255;
		if (c == 'q' || c == 'Q' || !f)
			break;
	}
	if (f)
		fclose(f);
	return;
}

int main()
{
	Train();

	Detect();

	return 0;
}
