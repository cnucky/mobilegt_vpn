/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   logger.h
 * Author: lenovo-pc
 *
 * Created on 2017年10月11日, 上午11:52
 */

#ifndef LOGGER_H
#define LOGGER_H

enum log_level {
	DEBUG, INFO, WARN, ERROR, FATAL
};

class CacheLogger {
public:
	CacheLogger();
	void log(log_level ll, string fun_name, string log_str);
	void start_log_service();
	void stop_log_service();
	log_level getLogLevel(string str_loglevel);
	string LOG_DIR = "log/";
	string logfileNameBase = "mobilegt_vpn_";
	log_level LOG_LEVEL_SET = log_level::FATAL;
	bool OPEN_DEBUGLOG = false;
	bool OPEN_COUT = false;
	string currentLogFile;
	int LOG_SIZE_MAX = 10 * 1024 * 1024;
	int loground[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	int cLoground = 0;

private:
	void hibernateLog(vector<string> *);
	vector<string> * switchlogcache();
	vector<string> * getCurrentCache();
	void checkLogger();
	vector<string> * currentCache;
	vector<string> vec_cache1;
	vector<string> vec_cache2;
	int hibernate_interval = 2000; //milliseconds
	int checkTimer=5;//每hiberante多少次检查一次log文件是否超出最大文件设置
	mutex mtx_file;
	bool STOP = false;
	int CACHE_SWITCH_SIZE;
	mutex mtx_logger;
	ofstream logger;
};


#endif /* LOGGER_H */

