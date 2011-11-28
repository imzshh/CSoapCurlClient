// SoapCurlClient.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <stdio.h>

#include "CSoapCurlClient.h"
#include "TestService.h"


int _tmain(int argc, _TCHAR* argv[])
{
    ::CoInitialize(NULL);

    TestService::CTestServiceT<CSoapCurlClient> service;

    BSTR helloResult;
    service.HelloWorld(&helloResult);
    printf("%s\n", CStringA(helloResult));

    BSTR nihaoResult;
    service.NiHaoShiJie(&nihaoResult);
    printf("%s\n", CStringA(nihaoResult));

	return 0;
}

