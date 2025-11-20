#pragma once
#include <opencv2/core/utils/logger.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>

class OpenCVSupportFormat {
public:
	static void PrintSupportFormat() {
		auto _backup = cv::utils::logging::getLogLevel();
		cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);
		std::vector<std::string> formats = {
			"bmp", "dib", "jpeg", "jpg", "jpe", "jp2", "png",
			"pbm", "pgm", "ppm", "sr", "ras", "tiff", "tif",
			"webp", "exr", "hdr", "pic"
		};

		std::vector<std::string> read_supported;
		std::vector<std::string> write_supported;

		for (const auto& ext : formats) {
			cv::Mat image = makeTestImage(ext);
			std::vector<uchar> buf;
			bool write_ok = false;

			try {
				write_ok = cv::imencode("." + ext, image, buf);
				if (write_ok) write_supported.push_back(ext);
			}
			catch (...) {
				continue; // 静默跳过
			}

			try {
				if (write_ok && !buf.empty()) {
					cv::Mat decoded = cv::imdecode(buf, cv::IMREAD_UNCHANGED);
					if (!decoded.empty()) read_supported.push_back(ext);
				}
			}
			catch (...) {
				continue; // 静默跳过
			}
		}

		std::cout << "Supported Read Formats:\n";
		printList(read_supported);
		std::cout << "\nSupported Write Formats:\n";
		printList(write_supported);
		cv::utils::logging::setLogLevel(_backup);
	}

private:
	static void printList(const std::vector<std::string>& list) {
		for (size_t i = 0; i < list.size(); ++i) {
			std::cout << list[i];
			if (i != list.size() - 1) std::cout << " | ";
		}
		std::cout << std::endl;
	}

	static cv::Mat makeTestImage(const std::string& ext) {
		if (ext == "pbm" || ext == "pgm")
			return cv::Mat(10, 10, CV_8UC1, cv::Scalar(255));
		else if (ext == "ppm")
			return cv::Mat(10, 10, CV_8UC3, cv::Scalar(255, 255, 255));
		else
			return cv::Mat(10, 10, CV_8UC3, cv::Scalar(255, 255, 255));
	}
};
