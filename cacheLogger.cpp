/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "mobilegt_util.h"
CacheLogger::CacheLogger() {
	currentCache = &vec_cache1;
}
void CacheLogger::log(log_level ll, string fun_name, string log_str) {
	if (ll >= LOG_LEVEL_SET) {
		auto logTime = chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		stringstream ss;
		ss << std::put_time(std::localtime(&logTime), "%F %T") << " ";
		switch (ll) {
			case DEBUG:
				ss << "DEBUG:";
				break;
			case INFO:
				ss << "INFO:";
				break;
			case WARN:
				ss << "WARN:";
				break;
			case ERROR:
				ss << "ERROR:";
				break;
			case FATAL:
				ss << "FATAL:";
				break;
		}
		ss << " [" << fun_name << "] " << log_str << endl;
		mtx_logger.lock(); //logger上锁
		currentCache->push_back(ss.str());
		mtx_logger.unlock();
	}
}
void CacheLogger::hibernateLog(vector<string> * logcache) {
	if (checkTimer <= 0) {
		checkTimer = 5;
		checkLogger();
	} else {
		checkTimer--;
	}
	mtx_file.lock();
	for (string str_log : *logcache) {
		cout << str_log;
		logger << str_log;
	}
	logger.flush();
	mtx_file.unlock();
}
void CacheLogger::start_log_service() {
	while (!STOP) {
		this_thread::sleep_for(chrono::milliseconds(hibernate_interval)); //default:2000 milliseconds
		if (currentCache->size() > 0) {
			vector<string> *logcache = switchlogcache();
			hibernateLog(logcache);
			logcache->clear();
		}
	}
	logger.close();
}
void CacheLogger::stop_log_service() {
	STOP = true;
}
vector<string> * CacheLogger::switchlogcache() {
	mtx_logger.lock();
	vector<string> *logcache = &vec_cache1;
	if (currentCache == &vec_cache1) {
		currentCache = &vec_cache2;
	} else {
		currentCache = &vec_cache1;
		logcache = &vec_cache2;
	}
	mtx_logger.unlock();
	return logcache;
}
vector<string> * CacheLogger::getCurrentCache() {
	return currentCache;
}

//// 
//// 该方法检测日志文件是过大,过大则round robin处理
//// 每个日志文件最大LOG_SIZE_MAX(缺省10*1024*1024即10M),超过则写入下一个日志文件
//// 
void CacheLogger::checkLogger() {
	mtx_file.lock();
	const string FUN_NAME = "CacheLogger-->checkLogger";
	//已分离出一个无锁版本的_log()方法
	//无需设置,checkLogger调用的方法log()方法是无锁版本的_log()方法
	//const bool CHECK_LOGGER = false; //必须设置为false,否则log()----checklogger()----log()死递归了
	if (!logger.is_open()) {
		logger.open((LOG_DIR + currentLogFile).c_str(), ios::trunc | ios::out);
		log(log_level::INFO, FUN_NAME, "openfile:" + LOG_DIR + currentLogFile);
	}
	struct stat buf;

	if (stat((LOG_DIR + currentLogFile).c_str(), &buf) < 0) {
		log(log_level::ERROR, FUN_NAME, "stat file failed:" + LOG_DIR + currentLogFile);
	} else {
		if (buf.st_size > LOG_SIZE_MAX) {
			log(log_level::DEBUG, FUN_NAME, LOG_DIR + currentLogFile + " file size is " + to_string(buf.st_size));
			cLoground++;
			cLoground %= 10;
			currentLogFile = logfileNameBase + "." + to_string(loground[cLoground]);
			logger.close();
			logger.open((LOG_DIR + currentLogFile).c_str(), ios::trunc | ios::out);
			log(log_level::DEBUG, FUN_NAME, "open new file " + LOG_DIR + currentLogFile);
		}
	}
	mtx_file.unlock();
}
//// 
//// 日志级别类型转换处理:字符串到enum类型处理
////
log_level CacheLogger::getLogLevel(string str_loglevel) {
	log_level ll = log_level::DEBUG;
	if (str_loglevel == "DEBUG")
		ll = log_level::DEBUG;
	else if (str_loglevel == "INFO")
		ll = log_level::INFO;
	else if (str_loglevel == "WARN")
		ll = log_level::WARN;
	else if (str_loglevel == "ERROR")
		ll = log_level::ERROR;
	else if (str_loglevel == "FATAL")
		ll = log_level::FATAL;

	return ll;
}