#pragma once
typedef enum EInterprocError
{
	INTERPROC_SUCCESS = 0,
	INTERPROC_ERROR_SEND = 100,
	INTERPROC_ERROR_RECEIVE = 200,
	INTERPROC_ERROR_OPEN = 300,
	INTERPROC_ERROR_CREATE = 400
};
