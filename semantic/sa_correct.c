/*
 * sa_correct.c
 *
 *  Created on: 2017年10月17日
 *      Author: listen
 */

#include <Python.h>

static PyObject * pModule = NULL;
static PyObject * pFunc0 = NULL;
static PyObject * pFunc = NULL;
static PyObject * pArgs = NULL;
static PyObject * pRetVal = NULL;
static PyObject * pArgs0 = NULL;


int correct_init(void)
{
	Py_Initialize();
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append('./correct/')");
	pModule = PyImport_ImportModule("ED_release");
	pFunc = PyObject_GetAttrString(pModule, "process");
	pFunc0 = PyObject_GetAttrString(pModule, "similarity");

	return 0;
}

void correct_free(void)
{
    Py_DECREF(pModule);
	Py_DECREF(pFunc0);
	Py_DECREF(pFunc);
	Py_DECREF(pArgs);
	Py_DECREF(pArgs0);
	Py_DECREF(pRetVal);
	Py_Finalize();
}


int similarity(char *db_path, char* src, char *result, float *distance)
{
	printf("coming in");
	if(db_path==NULL||src == NULL|| result == NULL){
		return -1;	
	}
	else if(db_path[0]=='\0'||strlen(src) <= 5){
		return -1;	
	}
//	float distance=0;
	char *tmp = NULL;
	pArgs = PyTuple_New(1);
	PyTuple_SetItem(pArgs, 0, Py_BuildValue("s", src));
	pRetVal = PyEval_CallObject(pFunc, pArgs);

	pArgs0 = PyTuple_New(2);
	PyTuple_SetItem(pArgs0, 0, pRetVal);
	PyTuple_SetItem(pArgs0, 1, Py_BuildValue("s", db_path));
	pRetVal = PyEval_CallObject(pFunc0, pArgs0);

	//printf("result：%s\n", tmp);
	PyArg_ParseTuple(pRetVal, "fs", distance, &tmp);
	printf("相似度为：%f\n", *distance);
	
	//printf("function return value : %s\n", PyString_AsString(pRetVal));
	if(*distance > 0.50)
	{
		//printf("result：%s\n", tmp);
		strcpy(result, tmp);
	}
	printf("function return value1 : %s\n", result);

	return 0;
}



