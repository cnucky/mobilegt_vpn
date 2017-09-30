/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   get_config_simpletest.cpp
 * Author: lenovo-pc
 *
 * Created on 2017年8月19日, 下午9:41
 */

#include <stdlib.h>
#include <iostream>
#include "get_config.h"

/*
 * Simple C++ Test Suite
 */

int test1() {
	std::cout << "get_config_simpletest test 1" << std::endl;
	map<string, string> m;
	ReadConfig("/home/rywang/test.cfg", m);
	PrintConfig(m);

	return 0;
}

void test2() {
	std::cout << "get_config_simpletest test 2" << std::endl;
	std::cout << "%TEST_FAILED% time=0 testname=test2 (get_config_simpletest) message=error message sample" << std::endl;
}

int main(int argc, char** argv) {
	std::cout << "%SUITE_STARTING% get_config_simpletest" << std::endl;
	std::cout << "%SUITE_STARTED%" << std::endl;

	std::cout << "%TEST_STARTED% test1 (get_config_simpletest)" << std::endl;
	test1();
	std::cout << "%TEST_FINISHED% time=0 test1 (get_config_simpletest)" << std::endl;

//	std::cout << "%TEST_STARTED% test2 (get_config_simpletest)\n" << std::endl;
//	test2();
//	std::cout << "%TEST_FINISHED% time=0 test2 (get_config_simpletest)" << std::endl;
//
//	std::cout << "%SUITE_FINISHED% time=0" << std::endl;

	return (EXIT_SUCCESS);
}

