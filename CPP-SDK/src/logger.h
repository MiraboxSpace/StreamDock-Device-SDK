#ifndef _LOGFILE_H_
#define _LOGFILE_H_
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <queue>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <fstream>
#include <atomic>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <ctime>
#include <iomanip>
#include <filesystem>
#include <chrono>
#include <filesystem>
namespace xiaochufuji {
	template <typename T>
	std::string to_string_helper(T&& arg)
	{
		std::ostringstream oss;
		oss << std::forward<T>(arg);
		return oss.str();
	}

	class LogQueue
	{
	public:
		void push(const std::string& msg)
		{
			std::lock_guard<std::mutex> lock(mutex_);
			queue_.push(msg);
			cond_var_.notify_one();
		}

		bool pop(std::string& msg)
		{
			std::unique_lock<std::mutex> lock(mutex_);
			cond_var_.wait(lock, [this]()
				{ return !queue_.empty() || is_shutdown_; });
			if (is_shutdown_ && queue_.empty())
			{
				return false;
			}

			msg = queue_.front();
			queue_.pop();
			return true;
		}

		void shutdown()
		{
			std::lock_guard<std::mutex> lock(mutex_);
			is_shutdown_ = true;
			cond_var_.notify_all();
		}

	private:
		std::queue<std::string> queue_;
		std::mutex mutex_;
		std::condition_variable cond_var_;
		bool is_shutdown_ = false;
	};
	enum class LogLevel
	{
		_INFO_,
		_DEBUG_,
		_WARN_,
		_ERROR_
	};

	class Logger
	{
	public:
		Logger() : exit_flag_(false)
		{
			// if dir existed ?
			namespace fs = std::filesystem;
			std::string logDir = "./log";
			if (!fs::exists(logDir))
			{
				fs::create_directories(logDir);
			}
			auto now = std::chrono::system_clock::now();
			std::time_t now_c = std::chrono::system_clock::to_time_t(now);
			std::tm tm{};
#ifdef _WIN32
			localtime_s(&tm, &now_c);
#else
			localtime_r(&now_c, &tm);
#endif
			std::ostringstream oss;
			oss << logDir << "/log_" << std::setfill('0')
				<< std::setw(2) << (tm.tm_year % 100)
				<< std::setw(2) << (tm.tm_mon + 1)
				<< std::setw(2) << tm.tm_mday
				<< ".txt";
			std::string logFilePath = oss.str();
			log_file_ = std::ofstream(logFilePath, std::ios::out | std::ios::app);
			if (!log_file_.is_open())
			{
				throw std::runtime_error("Failed to open log file");
			}

			worker_thread_ = std::thread([this]()
				{
					std::string msg;
					while (!exit_flag_ && log_queue_.pop(msg)) {
						log_file_ << msg << std::endl;
					} });
		}
		~Logger()
		{
			exit_flag_ = true;
			log_queue_.shutdown();
			if (worker_thread_.joinable())
			{
				worker_thread_.join();
			}

			if (log_file_.is_open())
			{
				log_file_.close();
			}
		}
		template <typename... Args>
		void log(LogLevel level, const std::string& format, Args &&...args)
		{
			std::string level_str;
			switch (level)
			{
			case LogLevel::_INFO_:
				level_str = "[*INFO]";
				break;
			case LogLevel::_DEBUG_:
				level_str = "[DEBUG]";
				break;
			case LogLevel::_WARN_:
				level_str = "[*WARN]";
				break;
			case LogLevel::_ERROR_:
				level_str = "[ERROR]";
				break;
			}

			log_queue_.push(level_str + formatMessage(format, std::forward<Args>(args)...));
		}

	private:
		std::string getCurrentTime()
		{
			auto now = std::chrono::system_clock::now();
			std::time_t time = std::chrono::system_clock::to_time_t(now);
			char buffer[100];
			std::tm tm_buf;
#ifdef _WIN32
			localtime_s(&tm_buf, &time);
#else
			localtime_r(&time, &tm_buf);
#endif

			std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm_buf);
			return std::string(buffer);
		}

		template <typename... Args>
		std::string formatMessage(const std::string& format, Args &&...args)
		{
			std::vector<std::string> arg_strings = { to_string_helper(std::forward<Args>(args))... };
			std::ostringstream oss;
			size_t arg_index = 0;
			size_t pos = 0;
			size_t placeholder = format.find("{}", pos);
			while (placeholder != std::string::npos)
			{
				oss << format.substr(pos, placeholder - pos);
				if (arg_index < arg_strings.size())
				{
					oss << arg_strings[arg_index++];
				}
				else
				{
					oss << "{}";
				}

				pos = placeholder + 2;
				placeholder = format.find("{}", pos);
			}

			oss << format.substr(pos);
			while (arg_index < arg_strings.size())
			{
				oss << arg_strings[arg_index++];
			}
			return "[" + getCurrentTime() + "] " + oss.str();
		}
		LogQueue log_queue_;
		std::thread worker_thread_;
		std::ofstream log_file_;
		std::atomic<bool> exit_flag_;
	};
	// class LogToFile
	//{
	// public:
	//	static void _LogToFile(const std::string &message, const std::string &srcFile, const int lineNumber, xiaochufuji::LogLevel level)
	//	{
	//		std::string fileBaseName = srcFile;
	//		size_t lastSlashPos = fileBaseName.find_last_of("/\\");
	//		if (lastSlashPos != std::string::npos)
	//		{
	//			fileBaseName = fileBaseName.substr(lastSlashPos + 1);
	//		}
	//		logger.log(level, "FILE: {}, LINE: {}, {}", fileBaseName, lineNumber, message);
	//	}

	// private:
	//	static Logger logger;
	// };
	// Logger LogToFile::logger;
}
// #define LOG_ERROR(message) xiaochufuji::LogToFile::_LogToFile(message, __FILE__, __LINE__, xiaochufuji::LogLevel::_ERROR_)
// #define LOG_INFO(message) xiaochufuji::LogToFile::_LogToFile(message, __FILE__, __LINE__, xiaochufuji::LogLevel::_INFO_)
// #define LOG_DEBUG(message) xiaochufuji::LogToFile::_LogToFile(message, __FILE__, __LINE__, xiaochufuji::LogLevel::_DEBUG_)
// #define LOG_WARN(message) xiaochufuji::LogToFile::_LogToFile(message, __FILE__, __LINE__, xiaochufuji::LogLevel::_WARN_)
//
//  int main() {
//     try {
//         Logger logger("log.txt");
//         logger.log(LogLevel::_INFO_, "Starting application...");
//         int user_id = 42;
//         std::string action = "login";
//         double duration = 3.5;
//         std::string world = "World";
//         logger.log(LogLevel::_INFO_, "User {} performed {} in {} seconds", user_id, action, duration);
//         logger.log(LogLevel::_DEBUG_, "Hello {}", world);
//         logger.log(LogLevel::_WARN_, "This is a message whithout any placeholders");
//         logger.log(LogLevel::_ERROR_, "Multiple placeholders : {}, {},", 1, 2, 3);
//     }
//     catch (const std::exception& e) {
//         std::cerr << "Error is " << e.what() << std::endl;
//     }
//
//     return 0;
// }
#endif
