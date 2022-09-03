#include "ARootTask.h"

TOSHI_CLASS_DERIVED_INITIALIZE(ARootTask, Toshi::TTask, MKVERSION(1, 1))

ARootTask::ARootTask()
{
	TOSHI_INFO("Called ARootTask");
}

ARootTask::~ARootTask()
{
	TOSHI_INFO("Called ~ARootTask");
}