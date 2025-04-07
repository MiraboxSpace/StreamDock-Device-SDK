#pragma once
#include "TranSport.h"
#include "hidapi.h"
#include <algorithm>
#include <vector>
#include "FreeImagePlus.h"
#define AUTOREAD true
enum class RgbaFormat
{
	GRAY = 1,
	RGB = 3,
	RGBA = 4
};

class StreamDock
{
	friend class PicHelper;

public:
	StreamDock(const std::shared_ptr<TranSport> &transport, struct hid_device_info *devInfo);
	virtual ~StreamDock();
	std::vector<unsigned char> getFirmVersion(int length);
	int open(bool auto_read = false);
	virtual void init(int brightness = 100);
	int disconnected();
	std::vector<unsigned char> read();
	static std::string parseRead(const std::vector<unsigned char>& readVec);
	int setBrightness(int percent);
	int refresh();
	std::string getPath();
	int wakeScreen();
	virtual int clearIcon(int index);
	virtual int clearAllIcon();
	// only override by StreamDockN1
	virtual int switchMode(N1MODE mode);

	/*  usage:
		s->setBackgroundImg("./img/bg.jpg");
		s->refresh();
	*/
	virtual int setBackgroundImg(const std::string &path) = 0;
	/*  usage:
		// if RGBA, set pitch 4; RGB, set 3; GRAY, set 1.
		// you should set the picture's width and height you load from disk, and we will rescale the picture to target width and height
		fipImage img;
		img.load("./img/bg.png");
		s->setBackgroundImgData(img.accessPixels(), RgbaFormat::RGB, 1234, 567); // we will rescale to 800 * 480
		s->refresh();
	*/
	virtual int setBackgroundImgData(unsigned char *imagedata, RgbaFormat pitch = RgbaFormat::RGB, unsigned int loadPicWidth = 0, unsigned int loadPicHeight = 0) = 0;
	/*  usage:
		s->setKeyImg("./img/tiga112.png", 8);
		s->refresh();
	*/
	virtual int setKeyImg(const std::string &path, int key) = 0;
	/*  usage:
		// if RGBA, set pitch 4; RGB, set 3; GRAY, set 1.
		// you should set the picture's width and height you load from disk, and we will rescale the picture to target width and height
		fipImage img;
		img.load("./img/tiga112.png");
		s->setKeyImgData(img.accessPixels(), 2, RgbaFormat::RGBA, 112, 112);
		s->refresh();
	*/
	virtual int setKeyImgData(unsigned char *imagedata, int key, RgbaFormat pitch = RgbaFormat::RGB, unsigned int loadPicWidth = 0, unsigned int loadPicHeight = 0) = 0;

protected:
	void setKeyFormat(unsigned int width, unsigned int height);
	void setKeyFormat_secondScreen(unsigned int width, unsigned int height);
	void setBgiFormat(unsigned int width, unsigned int height);
	void setRotate(float kyAngle, float bgAngle);
	void setFlipHorizonal(bool bgFlip, bool keyFlip);
	void setFlipVertical(bool bgFlip, bool keyFlip);
	void setKeyRange(unsigned int minKeyNum, unsigned int maxKeyNum);
	void setKeySecondScreenRange(unsigned int minKeyNum, unsigned int maxKeyNum);
	bool isKeyNormal(unsigned int key);
	bool isKeySecondScreen(unsigned int key);

public:
	std::shared_ptr<TranSport> transport;
	hid_device_info_store info;

protected:
	unsigned int bg_width = 800;
	unsigned int bg_height = 480;
	unsigned int ky_width = 100;
	unsigned int ky_height = 100;
	unsigned int ky_width_secondScreen = 100;
	unsigned int ky_height_secondScreen = 100;
	bool flipBgHorizonal = false;
	bool flipBgVertical = false;
	bool flipKyHorizonal = false;
	bool flipKyVertical = false;
	float kyRotateAngle = 0.0f;
	float bgRotateAngle = 0.0f;
	unsigned int maxKey = 0;
	unsigned int minKey = 0;
	unsigned int maxSecondKey = 0;
	unsigned int minSecondKey = 0;
};

class PicHelper
{
public:
	static void toNativeBackground(fipImage &img, StreamDock *dock);
	static void toNativeKeyImage(fipImage &img, StreamDock *dock);
	static void toNativeKeyImage_secondScreen(fipImage& img, StreamDock* dock);
};

class joining_thread {
	std::thread  _t;
public:
	joining_thread() noexcept = default;
	template<typename Callable, typename ...  Args>
	explicit  joining_thread(Callable&& func, Args&& ...args) :
		_t(std::forward<Callable>(func), std::forward<Args>(args)...) {
	}
	explicit joining_thread(std::thread t) noexcept : _t(std::move(t)) {}
	joining_thread(joining_thread&& other) noexcept : _t(std::move(other._t)) {}
	joining_thread& operator=(joining_thread&& other) noexcept
	{
		if (joinable()) {
			join();
		}
		_t = std::move(other._t);
		return *this;
	}
	~joining_thread() noexcept {
		if (joinable()) {
			join();
		}
	}
	void swap(joining_thread& other) noexcept {
		_t.swap(other._t);
	}
	std::thread::id   get_id() const noexcept {
		return _t.get_id();
	}
	bool joinable() const noexcept {
		return _t.joinable();
	}
	void join() {
		_t.join();
	}
	void detach() {
		_t.detach();
	}
	std::thread& as_thread() noexcept {
		return _t;
	}
	const std::thread& as_thread() const noexcept {
		return _t;
	}
};