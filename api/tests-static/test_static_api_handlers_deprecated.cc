/************************************************************************************************************************
 Copyright (c) 2016, Imagination Technologies Limited and/or its affiliated group companies.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
 following conditions are met:
     1. Redistributions of source code must retain the above copyright notice, this list of conditions and the
        following disclaimer.
     2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
        following disclaimer in the documentation and/or other materials provided with the distribution.
     3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote
        products derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
 USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
************************************************************************************************************************/

// NOTE: this file uses deprecated API functions.

#include <pthread.h>
#include <gtest/gtest.h>
#include "awa/static.h"
#include "awa/server.h"
#include "support/static_api_support.h"

// reverse the name and objectID to match updated API:
#define AwaStaticClient_DefineObject(A, B, C, D, E) AwaStaticClient_DefineObject(A, C, B, D, E)
#define AwaStaticClient_DefineResource(A, B, C, D, E, F, G, H) AwaStaticClient_DefineResource(A, C, D, B, E, F, G, H)

namespace Awa {

class TestStaticClientHandlerWithServerDeprecated : public TestStaticClientWithServer {};

TEST_F(TestStaticClientHandlerWithServerDeprecated, AwaStaticClient_Create_and_Write_Operation_for_Object_and_Resource)
{
    struct callback1 : public StaticClientCallbackPollCondition
    {
        AwaInteger integer = 5;

        callback1(AwaStaticClient * StaticClient, int maxCount) : StaticClientCallbackPollCondition(StaticClient, maxCount) {};

        AwaResult handler(AwaStaticClient * context, AwaOperation operation, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, void ** dataPointer, size_t * dataSize, bool * changed)
        {

            AwaResult result = AwaResult_InternalError;
            EXPECT_TRUE((operation == AwaOperation_CreateResource) || (operation == AwaOperation_CreateObjectInstance) || (operation == AwaOperation_Write) || (operation == AwaOperation_Read));

            switch (operation)
            {
                case AwaOperation_CreateObjectInstance:
                {
                    EXPECT_EQ(9999, objectID);
                    EXPECT_EQ(0, objectInstanceID);
                    result = AwaResult_SuccessCreated;
                    break;
                }
                case AwaOperation_CreateResource:
                {
                    EXPECT_EQ(9999, objectID);
                    EXPECT_EQ(0, objectInstanceID);
                    EXPECT_EQ(1, resourceID);
                    result = AwaResult_SuccessCreated;
                    break;
                }
                case AwaOperation_Write:
                {
                    EXPECT_TRUE(dataPointer != NULL);
                    AwaInteger * integerPtr = (AwaInteger *)(*dataPointer);
                    EXPECT_TRUE(dataSize != NULL);
                    EXPECT_EQ(static_cast<size_t>(sizeof(AwaInteger)), *dataSize);
                    EXPECT_EQ(5, *integerPtr);
                    complete = true;
                    result = AwaResult_SuccessChanged;
                    break;
                }
                case AwaOperation_Read:
                {
                    EXPECT_TRUE(dataPointer != NULL);
                    EXPECT_TRUE(dataSize != NULL);
                    *dataPointer = &integer;
                    *dataSize = sizeof(integer);
                    result = AwaResult_SuccessContent;
                    break;
                }
                default:
                    result = AwaResult_InternalError;
                    break;
            }

            return result;
        }
    };

    callback1 cbHandler(client_, 20);
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetApplicationContext(client_, &cbHandler));
    EXPECT_EQ(AwaError_Success,AwaStaticClient_DefineObjectWithHandler(client_, "TestObject", 9999, 0, 1, handler));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineResourceWithHandler(client_, "TestResource", 9999, 1, AwaResourceType_Integer, 1, 1, AwaResourceOperations_ReadWrite, handler));

    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_TRUE(NULL != operation);
    SingleStaticClientPollCondition condition(client_, operation, global::clientEndpointName, 10);
    EXPECT_TRUE(condition.Wait());
    AwaServerListClientsOperation_Free(&operation);

    AwaServerDefineOperation * defineOperation = AwaServerDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);
    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(9999, "TestObject", 0, 1);
    EXPECT_TRUE(objectDefinition != NULL);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(objectDefinition, 1, "TestResource", true, AwaResourceOperations_ReadWrite, 0));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(defineOperation, objectDefinition));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(defineOperation, defaults::timeout));
    AwaServerDefineOperation_Free(&defineOperation);
    AwaObjectDefinition_Free(&objectDefinition);

    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
    EXPECT_TRUE(writeOperation != NULL);
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/9999/0"));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, "/9999/0/1", 5));

    pthread_t t;
    pthread_create(&t, NULL, do_write_operation, (void *)writeOperation);
    ASSERT_TRUE(cbHandler.Wait());
    pthread_join(t, NULL);

    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestStaticClientHandlerWithServerDeprecated, AwaStaticClient_Create_and_Read_Operation_for_Object_and_Resource)
{
    struct callback1 : public StaticClientCallbackPollCondition
    {
        AwaInteger integer = 5;
        int counter = 0;

        callback1(AwaStaticClient * StaticClient, int maxCount) : StaticClientCallbackPollCondition(StaticClient, maxCount) {};

        AwaResult handler(AwaStaticClient * context, AwaOperation operation, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, void ** dataPointer, size_t * dataSize, bool * changed)
        {
            AwaResult result = AwaResult_InternalError;
            EXPECT_TRUE((operation == AwaOperation_CreateResource) || (operation == AwaOperation_CreateObjectInstance) || (operation == AwaOperation_Read));

            switch(operation)
            {
                case AwaOperation_CreateObjectInstance:
                {
                    EXPECT_EQ(9999, objectID);
                    EXPECT_EQ(0, objectInstanceID);
                    result = AwaResult_SuccessCreated;
                    break;
                }
                case AwaOperation_CreateResource:
                {
                    EXPECT_EQ(9999, objectID);
                    EXPECT_EQ(0, objectInstanceID);
                    EXPECT_EQ(1, resourceID);
                    result = AwaResult_SuccessCreated;
                    break;
                }
                case AwaOperation_Read:
                {
                    EXPECT_TRUE(dataPointer != NULL);
                    EXPECT_TRUE(dataSize != NULL);
                    *dataPointer = &integer;
                    *dataSize = sizeof(integer);
                    complete = counter++ == 1 ? true : false;
                    result = AwaResult_SuccessContent;
                    break;
                }
                default:
                    result = AwaResult_InternalError;
                    break;
            }

            return result;
        }
    };

    callback1 cbHandler(client_, 20);

    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetApplicationContext(client_, &cbHandler));
    EXPECT_EQ(AwaError_Success,AwaStaticClient_DefineObjectWithHandler(client_, "TestObject", 9999, 0, 1, handler));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineResourceWithHandler(client_, "TestResource", 9999, 1, AwaResourceType_Integer, 1, 1, AwaResourceOperations_ReadWrite, handler));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_CreateObjectInstance(client_, 9999, 0));

    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_TRUE(NULL != operation);
    SingleStaticClientPollCondition condition(client_, operation, global::clientEndpointName, 10);
    EXPECT_TRUE(condition.Wait());
    AwaServerListClientsOperation_Free(&operation);

    AwaServerDefineOperation * defineOperation = AwaServerDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);
    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(9999, "TestObject", 0, 1);
    EXPECT_TRUE(objectDefinition != NULL);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(objectDefinition, 1, "TestResource", true, AwaResourceOperations_ReadWrite, 0));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(defineOperation, objectDefinition));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(defineOperation, defaults::timeout));
    AwaServerDefineOperation_Free(&defineOperation);
    AwaObjectDefinition_Free(&objectDefinition);

    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(session_);
    EXPECT_TRUE(readOperation != NULL);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/9999/0"));

    pthread_t t;
    pthread_create(&t, NULL, do_read_operation, (void *)readOperation);
    ASSERT_TRUE(cbHandler.Wait());
    pthread_join(t, NULL);

    AwaServerReadOperation_Free(&readOperation);
}

static void * do_delete_operation(void * attr)
{
    AwaServerDeleteOperation * deleteOperation = (AwaServerDeleteOperation *)attr;
    AwaServerDeleteOperation_Perform(deleteOperation, defaults::timeout);
    return 0;
}

TEST_F(TestStaticClientHandlerWithServerDeprecated, AwaStaticClient_Create_and_Delete_Operation_for_Object_and_Resource)
{
    struct callback1 : public StaticClientCallbackPollCondition
    {
        AwaInteger integer = 5;
        callback1(AwaStaticClient * StaticClient, int maxCount) : StaticClientCallbackPollCondition(StaticClient, maxCount) {};

        AwaResult handler(AwaStaticClient * context, AwaOperation operation, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, void ** dataPointer, size_t * dataSize, bool * changed)

        {
            AwaResult result = AwaResult_InternalError;
            EXPECT_TRUE((operation == AwaOperation_CreateResource) || (operation == AwaOperation_CreateObjectInstance) || (operation == AwaOperation_DeleteObjectInstance) || (operation == AwaOperation_Read));

            switch(operation)
            {

                case AwaOperation_CreateObjectInstance:
                case AwaOperation_DeleteObjectInstance:
                {
                    EXPECT_EQ(9999, objectID);
                    EXPECT_EQ(0, objectInstanceID);
                    result = operation == AwaOperation_CreateObjectInstance ? AwaResult_SuccessCreated : AwaResult_SuccessDeleted;

                    if (operation == AwaOperation_DeleteObjectInstance)
                        complete = true;

                    break;
                }
                case AwaOperation_CreateResource:
                {
                    EXPECT_EQ(9999, objectID);
                    EXPECT_EQ(0, objectInstanceID);
                    EXPECT_EQ(1, resourceID);
                    result = AwaResult_SuccessCreated;
                    break;
                }
                case AwaOperation_Read:
                {
                    EXPECT_TRUE(dataPointer != NULL);
                    EXPECT_TRUE(dataSize != NULL);
                    *dataPointer = &integer;
                    *dataSize = sizeof(integer);
                    result = AwaResult_SuccessContent;
                    break;
                }
                default:
                    result = AwaResult_InternalError;
                    break;
            }

            return result;
        }
    };

    callback1 cbHandler(client_, 20);
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetApplicationContext(client_, &cbHandler));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineObjectWithHandler(client_, "TestObject", 9999, 0, 1, handler));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineResourceWithHandler(client_, "TestResource", 9999, 1, AwaResourceType_Integer, 0, 1, AwaResourceOperations_ReadWrite, handler));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_CreateObjectInstance(client_, 9999, 0));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_CreateResource(client_, 9999, 0, 1));

    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_TRUE(NULL != operation);
    SingleStaticClientPollCondition condition(client_, operation, global::clientEndpointName, 10);
    EXPECT_TRUE(condition.Wait());
    AwaServerListClientsOperation_Free(&operation);

    AwaServerDefineOperation * defineOperation = AwaServerDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);
    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(9999, "TestObject", 0, 1);
    EXPECT_TRUE(objectDefinition != NULL);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(objectDefinition, 1, "TestResource", false, AwaResourceOperations_ReadWrite, 0));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(defineOperation, objectDefinition));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(defineOperation, defaults::timeout));
    AwaServerDefineOperation_Free(&defineOperation);
    AwaObjectDefinition_Free(&objectDefinition);

    AwaServerDeleteOperation * deleteOperation = AwaServerDeleteOperation_New(session_);
    EXPECT_TRUE(deleteOperation != NULL);
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(deleteOperation, global::clientEndpointName, "/9999/0"));

    pthread_t t;
    pthread_create(&t, NULL, do_delete_operation, (void *)deleteOperation);
    ASSERT_TRUE(cbHandler.Wait());
    pthread_join(t, NULL);

    AwaServerDeleteOperation_Free(&deleteOperation);
}

static void * do_execute_operation(void * attr)
{
    AwaServerExecuteOperation * executeOperation = (AwaServerExecuteOperation *)attr;
    AwaError result = AwaServerExecuteOperation_Perform(executeOperation, defaults::timeout);
    Lwm2m_Debug("AwaServerExecuteOperation_Perform: %s\n", AwaError_ToString(result));
    return 0;
}

TEST_F(TestStaticClientHandlerWithServerDeprecated, AwaStaticClient_Create_and_Execute_Operation_for_Object_and_Resource)
{
    struct callback1 : public StaticClientCallbackPollCondition
    {
        callback1(AwaStaticClient * StaticClient, int maxCount) : StaticClientCallbackPollCondition(StaticClient, maxCount) {};

        AwaResult handler(AwaStaticClient * context, AwaOperation operation, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, void ** dataPointer, size_t * dataSize, bool * changed)
        {
            AwaResult result = AwaResult_InternalError;

            EXPECT_TRUE((operation == AwaOperation_CreateResource) || (operation == AwaOperation_CreateObjectInstance) || (operation == AwaOperation_Execute));

            switch(operation)
            {
                case AwaOperation_CreateObjectInstance:
                case AwaOperation_DeleteObjectInstance:
                {
                    EXPECT_EQ(9999, objectID);
                    EXPECT_EQ(0, objectInstanceID);
                    result = operation == AwaOperation_CreateObjectInstance ? AwaResult_SuccessCreated : AwaResult_SuccessDeleted;
                    break;
                }
                case AwaOperation_CreateResource:
                {
                    EXPECT_EQ(9999, objectID);
                    EXPECT_EQ(0, objectInstanceID);
                    EXPECT_EQ(1, resourceID);
                    result = AwaResult_SuccessCreated;
                    break;
                }
                case AwaOperation_Execute:
                {
                    EXPECT_EQ(9999, objectID);
                    EXPECT_EQ(0, objectInstanceID);
                    EXPECT_EQ(1, resourceID);
                    result = AwaResult_Success;
                    complete = true;
                    break;
                }
                default:
                    result = AwaResult_InternalError;
                    break;
            }

            return result;
        }
    };

    callback1 cbHandler(client_, 20);
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetApplicationContext(client_, &cbHandler));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineObjectWithHandler(client_, "TestObject", 9999, 0, 1, handler));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineResourceWithHandler(client_, "TestResource", 9999, 1, AwaResourceType_None, 1, 1, AwaResourceOperations_Execute, handler));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_CreateObjectInstance(client_, 9999, 0));

    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_TRUE(NULL != operation);
    SingleStaticClientPollCondition condition(client_, operation, global::clientEndpointName, 10);
    EXPECT_TRUE(condition.Wait());
    AwaServerListClientsOperation_Free(&operation);

    AwaServerDefineOperation * defineOperation = AwaServerDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);
    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(9999, "TestObject", 0, 1);
    EXPECT_TRUE(objectDefinition != NULL);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsNoType(objectDefinition, 1, "TestExecutableResource", true, AwaResourceOperations_Execute));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(defineOperation, objectDefinition));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(defineOperation, defaults::timeout));
    AwaServerDefineOperation_Free(&defineOperation);
    AwaObjectDefinition_Free(&objectDefinition);

    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session_);
    EXPECT_TRUE(executeOperation != NULL);
    EXPECT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/9999/0/1", NULL));

    pthread_t t;
    pthread_create(&t, NULL, do_execute_operation, (void *)executeOperation);
    ASSERT_TRUE(cbHandler.Wait());
    pthread_join(t, NULL);

    AwaServerExecuteOperation_Free(&executeOperation);
}


namespace writeDetailDeprecated
{

struct TestWriteResourceDeprecated
{
    AwaError ExpectedAddResult;
    AwaError ExpectedProcessResult;

    AwaObjectID ObjectID;
    AwaObjectInstanceID ObjectInstanceID;
    AwaResourceID ResourceID;

    const void * Value;
    const size_t ValueCount;
    AwaResourceType Type;

    bool UseOperation;
};

::std::ostream& operator<<(::std::ostream& os, const TestWriteResourceDeprecated& item)
{
  return os << "Item: ExpectedAddResult " << item.ExpectedAddResult
            << ", ExpectedProcessResult " << item.ExpectedProcessResult
            << ", ObjectID " << item.ObjectID
            << ", ObjectInstanceID " << item.ObjectInstanceID
            << ", ResourceID " << item.ResourceID
            << ", Value " << item.Value
            << ", ValueCount " << item.ValueCount
            << ", Type " << item.Type
            << ", UseOperation " << item.UseOperation;
}

const char * clientID = "TestClient1";

static AwaInteger dummyInteger1 = 123456;
static const char * dummyString1 = "Lightweight M2M Server";
static AwaFloat dummyFloat1 = 1.0;
static AwaTime dummyTime1 = 0xA20AD72B;
static AwaBoolean dummyBoolean1 = true;
static char dummyOpaqueData[] = {'a',0,'x','\0', 123};
//static int dummyObjLinkData[] = {-1,-1};

AwaObjectLink dummyObjectLink1 = { 3, 5 };

const char * dummyStringArray1[] = {"Lightweight M2M Server", "test1", ""};
const char * dummyStringArray2[] = {"Lightweight M2M Server", "test1", "", "", "", ""};
const AwaInteger dummyIntegerArray1[] = {55, 8732, 11};
const AwaInteger dummyIntegerArray2[] = {55, 8732, 11, 55, 8732, 11};
const AwaFloat dummyFloatArray1[] = {55.0, 0.0008732, 11e10};
const AwaFloat dummyFloatArray2[] = {55.0, 0.0008732, 11e10, 55.0, 0.0008732, 11e10};
const AwaBoolean dummyBooleanArray1[] = {true, false, true};
const AwaBoolean dummyBooleanArray2[] = {true, false, true, true, false, true};

const AwaInteger dummyTimeArray1[] = {16000, 8732222, 1111};
const AwaInteger dummyTimeArray2[] = {16000, 8732222, 1111, 16000, 8732222, 1111};

const AwaObjectID TEST_OBJECT_NON_ARRAY_TYPES = 10000;
const AwaResourceID TEST_RESOURCE_STRING = 1;
const AwaResourceID TEST_RESOURCE_INTEGER = 2;
const AwaResourceID TEST_RESOURCE_FLOAT = 3;
const AwaResourceID TEST_RESOURCE_BOOLEAN = 4;
const AwaResourceID TEST_RESOURCE_OPAQUE = 5;
const AwaResourceID TEST_RESOURCE_TIME = 6;
const AwaResourceID TEST_RESOURCE_OBJECTLINK = 7;

const AwaObjectID TEST_OBJECT_ARRAY_TYPES = 10001;
const AwaResourceID TEST_RESOURCE_STRINGARRAY = 1;
const AwaResourceID TEST_RESOURCE_INTEGERARRAY = 2;
const AwaResourceID TEST_RESOURCE_FLOATARRAY = 3;
const AwaResourceID TEST_RESOURCE_BOOLEANARRAY = 4;
const AwaResourceID TEST_RESOURCE_OPAQUEARRAY = 5;
const AwaResourceID TEST_RESOURCE_TIMEARRAY = 6;
const AwaResourceID TEST_RESOURCE_OBJECTLINKARRAY = 7;

}

typedef AwaResult (*TestHandler)(void * context, AwaOperation operation, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, void ** dataPointer, size_t * dataSize, bool * changed);

struct TestWriteReadStaticResourceDeprecated
{
    TestHandler WriteHandler;
    TestHandler ReadHandler;

    AwaObjectID ObjectID;
    AwaObjectInstanceID ObjectInstanceID;
    AwaResourceID ResourceID;

    const void * Value;
    const int ValueCount;
    const int ValueSize;
    AwaResourceType Type;

    bool Complete;
    bool TestRead;
};

::std::ostream& operator<<(::std::ostream& os, const TestWriteReadStaticResourceDeprecated& item)
{
  return os << "Item: WriteHandler " << item.WriteHandler
            << ", ReadHandler " << item.ReadHandler
            << ", ObjectID " << item.ObjectID
            << ", ObjectInstanceID " << item.ObjectInstanceID
            << ", ResourceID " << item.ResourceID
            << ", Value " << item.Value
            << ", ValueCount " << item.ValueCount
            << ", ValueSize " << item.ValueSize
            << ", Type " << item.Type
            << ", Complete " << item.Complete
            << ", TestRead " << item.TestRead;
}

class TestStaticClientHandlerWriteReadValueDeprecated : public TestStaticClientWithServer, public ::testing::WithParamInterface< TestWriteReadStaticResourceDeprecated >
{

protected:

    class callback1 : public StaticClientCallbackPollCondition
    {
    public:

        TestWriteReadStaticResourceDeprecated data;

        callback1(AwaStaticClient * StaticClient, int maxCount, TestWriteReadStaticResourceDeprecated data) : StaticClientCallbackPollCondition(StaticClient, maxCount), data(data) {};

        AwaResult handler(AwaStaticClient * context, AwaOperation operation, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, void ** dataPointer, size_t * dataSize, bool * changed)
        {
            AwaResult result = AwaResult_InternalError;

            switch (operation)
            {
                    case AwaOperation_CreateObjectInstance:
                    {
                        EXPECT_EQ(data.ObjectID, objectID);
                        EXPECT_EQ(data.ObjectInstanceID, objectInstanceID);
                        result = AwaResult_SuccessCreated;
                        break;
                    }
                    case AwaOperation_CreateResource:
                    {
                        EXPECT_EQ(data.ObjectID, objectID);
                        EXPECT_EQ(data.ObjectInstanceID, objectInstanceID);
                        EXPECT_EQ(data.ResourceID, resourceID);
                        result = AwaResult_SuccessCreated;
                        break;
                    }
                    case AwaOperation_Write:
                        result = data.WriteHandler(&data, operation, objectID, objectInstanceID, resourceID, resourceInstanceID, dataPointer, dataSize, changed);
                        break;
                    case AwaOperation_Read:
                        result = data.ReadHandler(&data, operation, objectID, objectInstanceID, resourceID, resourceInstanceID, dataPointer, dataSize, changed);
                        break;
                    default:
                        break;
            }


            complete = data.Complete;
            return result;
        }
    };

    callback1 * cbHandler;


    void SetUp() {
        TestStaticClientWithServer::SetUp();
        TestWriteReadStaticResourceDeprecated data = GetParam();

        cbHandler = new callback1(client_, 20, data);

        EXPECT_EQ(AwaError_Success, AwaStaticClient_SetApplicationContext(client_, cbHandler));
        EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineObjectWithHandler(client_, "Test Object Single", writeDetailDeprecated::TEST_OBJECT_NON_ARRAY_TYPES, 0, 1, handler));
        EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineResourceWithHandler(client_, "Test String Resource",      writeDetailDeprecated::TEST_OBJECT_NON_ARRAY_TYPES, writeDetailDeprecated::TEST_RESOURCE_STRING,     AwaResourceType_String,     0, 1, AwaResourceOperations_ReadWrite, handler));
        EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineResourceWithHandler(client_, "Test Integer Resource",     writeDetailDeprecated::TEST_OBJECT_NON_ARRAY_TYPES, writeDetailDeprecated::TEST_RESOURCE_INTEGER,    AwaResourceType_Integer,    0, 1, AwaResourceOperations_ReadWrite, handler));
        EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineResourceWithHandler(client_, "Test Float Resource",       writeDetailDeprecated::TEST_OBJECT_NON_ARRAY_TYPES, writeDetailDeprecated::TEST_RESOURCE_FLOAT,      AwaResourceType_Float,      0, 1, AwaResourceOperations_ReadWrite, handler));
        EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineResourceWithHandler(client_, "Test Boolean Resource",     writeDetailDeprecated::TEST_OBJECT_NON_ARRAY_TYPES, writeDetailDeprecated::TEST_RESOURCE_BOOLEAN,    AwaResourceType_Boolean,    0, 1, AwaResourceOperations_ReadWrite, handler));
        EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineResourceWithHandler(client_, "Test Opaque Resource",      writeDetailDeprecated::TEST_OBJECT_NON_ARRAY_TYPES, writeDetailDeprecated::TEST_RESOURCE_OPAQUE,     AwaResourceType_Opaque,     0, 1, AwaResourceOperations_ReadWrite, handler));
        EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineResourceWithHandler(client_, "Test Time Resource",        writeDetailDeprecated::TEST_OBJECT_NON_ARRAY_TYPES, writeDetailDeprecated::TEST_RESOURCE_TIME,       AwaResourceType_Time,       0, 1, AwaResourceOperations_ReadWrite, handler));
        EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineResourceWithHandler(client_, "Test Object Link Resource", writeDetailDeprecated::TEST_OBJECT_NON_ARRAY_TYPES, writeDetailDeprecated::TEST_RESOURCE_OBJECTLINK, AwaResourceType_ObjectLink, 0, 1, AwaResourceOperations_ReadWrite, handler));
        EXPECT_EQ(AwaError_Success, AwaStaticClient_CreateObjectInstance(client_, writeDetailDeprecated::TEST_OBJECT_NON_ARRAY_TYPES, 0));

        AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
        EXPECT_TRUE(NULL != operation);
        SingleStaticClientPollCondition condition(client_, operation, global::clientEndpointName, 10);
        EXPECT_TRUE(condition.Wait());
        AwaServerListClientsOperation_Free(&operation);

        AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(session_);
        EXPECT_TRUE(serverDefineOperation != NULL);

        AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(writeDetailDeprecated::TEST_OBJECT_NON_ARRAY_TYPES, "Test Object Single", 0, 1);
        EXPECT_TRUE(NULL != customObjectDefinition);

        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsString     (customObjectDefinition, writeDetailDeprecated::TEST_RESOURCE_STRING,     "Test String Resource",      false, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger    (customObjectDefinition, writeDetailDeprecated::TEST_RESOURCE_INTEGER,    "Test Integer Resource",     false, AwaResourceOperations_ReadWrite, 0));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloat      (customObjectDefinition, writeDetailDeprecated::TEST_RESOURCE_FLOAT,      "Test Float Resource",       false, AwaResourceOperations_ReadWrite, 0.0));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBoolean    (customObjectDefinition, writeDetailDeprecated::TEST_RESOURCE_BOOLEAN,    "Test Boolean Resource",     false, AwaResourceOperations_ReadWrite, false));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque     (customObjectDefinition, writeDetailDeprecated::TEST_RESOURCE_OPAQUE,     "Test Opaque Resource",      false, AwaResourceOperations_ReadWrite, AwaOpaque {0}));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTime       (customObjectDefinition, writeDetailDeprecated::TEST_RESOURCE_TIME,       "Test Time Resource",        false, AwaResourceOperations_ReadWrite, 0));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLink (customObjectDefinition, writeDetailDeprecated::TEST_RESOURCE_OBJECTLINK, "Test Object Link Resource", false, AwaResourceOperations_ReadWrite, AwaObjectLink {0}));

        EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
        AwaObjectDefinition_Free(&customObjectDefinition);

        customObjectDefinition = AwaObjectDefinition_New(writeDetailDeprecated::TEST_OBJECT_ARRAY_TYPES, "Test Object Array", 0, 1);
        EXPECT_TRUE(NULL != customObjectDefinition);

        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsStringArray    (customObjectDefinition, writeDetailDeprecated::TEST_RESOURCE_STRING,     "Test String Array Resource",      0,5, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsIntegerArray   (customObjectDefinition, writeDetailDeprecated::TEST_RESOURCE_INTEGER,    "Test Integer Array Resource",     0,5, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloatArray     (customObjectDefinition, writeDetailDeprecated::TEST_RESOURCE_FLOAT,      "Test Float Array Resource",       0,5, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBooleanArray   (customObjectDefinition, writeDetailDeprecated::TEST_RESOURCE_BOOLEAN,    "Test Boolean Array Resource",     0,5, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaqueArray    (customObjectDefinition, writeDetailDeprecated::TEST_RESOURCE_OPAQUE,     "Test Opaque Array Resource",      0,5, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTimeArray      (customObjectDefinition, writeDetailDeprecated::TEST_RESOURCE_TIME,       "Test Time Array Resource",        0,5, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLinkArray(customObjectDefinition, writeDetailDeprecated::TEST_RESOURCE_OBJECTLINK, "Test Object Link Array Resource", 0,5, AwaResourceOperations_ReadWrite, NULL));

        EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
        EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, defaults::timeout));

        AwaObjectDefinition_Free(&customObjectDefinition);
        AwaServerDefineOperation_Free(&serverDefineOperation);

        writeOperation_ = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
        EXPECT_TRUE(NULL != writeOperation_);

        readOperation_ = AwaServerReadOperation_New(session_);
        EXPECT_TRUE(NULL != readOperation_);
    }

    void TearDown() {
        AwaServerWriteOperation_Free(&writeOperation_);
        AwaServerReadOperation_Free(&readOperation_);
        delete cbHandler;
        TestStaticClientWithServer::TearDown();
    }

    AwaClientSession * clientSession_;
    AwaServerSession * serverSession_;
    AwaClientDaemonHorde * horde_;
    AwaServerWriteOperation * writeOperation_;
    AwaServerReadOperation * readOperation_;
};

static AwaResult TestWriteValueStaticClient_WriteHandler(void * context, AwaOperation operation, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, void ** dataPointer, size_t * dataSize, bool * changed)
{
    TestWriteReadStaticResourceDeprecated * data = static_cast<TestWriteReadStaticResourceDeprecated *>(context);
    EXPECT_EQ(AwaOperation_Write, operation);
    EXPECT_EQ(data->ObjectID, objectID);
    EXPECT_EQ(data->ObjectInstanceID, objectInstanceID);
    EXPECT_EQ(data->ResourceID, resourceID);
    EXPECT_EQ(0, resourceInstanceID);

    EXPECT_TRUE(dataSize != NULL);
    EXPECT_TRUE(dataPointer != NULL);

    EXPECT_EQ(static_cast<size_t>(data->ValueSize), *dataSize);
    EXPECT_EQ(0, memcmp(data->Value, *dataPointer, data->ValueSize));

    data->Complete = true;
    return AwaResult_SuccessChanged;
}


static AwaResult TestWriteValueStaticClient_ReadHandler(void * context, AwaOperation operation, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, void ** dataPointer, size_t * dataSize, bool * changed)
{
    TestWriteReadStaticResourceDeprecated * data = static_cast<TestWriteReadStaticResourceDeprecated *>(context);

    EXPECT_EQ(AwaOperation_Read, operation);
    EXPECT_EQ(data->ObjectID, objectID);
    EXPECT_EQ(data->ObjectInstanceID, objectInstanceID);
    EXPECT_EQ(data->ResourceID, resourceID);
    EXPECT_EQ(0, resourceInstanceID);

    EXPECT_TRUE(dataSize != NULL);
    EXPECT_TRUE(dataPointer != NULL);

    *dataPointer = (void*)data->Value;
    *dataSize = data->ValueSize;

    if(data->TestRead == true)
    {
        data->Complete = true;
    }

    return AwaResult_SuccessContent;
}

TEST_P(TestStaticClientHandlerWriteReadValueDeprecated, TestWriteReadValueSingle)
{
    TestWriteReadStaticResourceDeprecated data = GetParam();
    char path[128] = {0};

    EXPECT_EQ(AwaError_Success, AwaAPI_MakePath(path, sizeof(path), data.ObjectID, data.ObjectInstanceID, data.ResourceID));

    switch (data.Type)
    {
        case AwaResourceType_String:
            EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsCString(writeOperation_, path, (const char *)data.Value));
            break;
        case AwaResourceType_Integer:
            EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation_, path, *((AwaInteger*)data.Value)));
            break;
        case AwaResourceType_Float:
            EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsFloat(writeOperation_, path, *((AwaFloat*)data.Value)));
            break;
        case AwaResourceType_Boolean:
            EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsBoolean(writeOperation_, path, *((AwaBoolean*)data.Value)));
            break;
        case AwaResourceType_Opaque:
            {
                AwaOpaque opaque = { (void *)data.Value, static_cast<size_t>(data.ValueSize) };
                EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsOpaque(writeOperation_, path, opaque));
                break;
            }
        case AwaResourceType_Time:
            EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsTime(writeOperation_, path, *((AwaTime*)data.Value)));
            break;
        case AwaResourceType_ObjectLink:
            {
                AwaObjectLink * objectlink = (AwaObjectLink *)data.Value;
                EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsObjectLink(writeOperation_, path, *objectlink));
                break;
            }
        default:
            EXPECT_TRUE(false);
            break;
    }

    pthread_t writeThread;
    pthread_create(&writeThread, NULL, do_write_operation, (void *)writeOperation_);
    ASSERT_TRUE(cbHandler->Wait());
    pthread_join(writeThread, NULL);

    cbHandler->data.Complete = false;
    cbHandler->data.TestRead = true;
    cbHandler->Reset();

    AwaServerReadOperation_AddPath(readOperation_, global::clientEndpointName, path);

    pthread_t readThread;
    pthread_create(&readThread, NULL, do_read_operation, (void *)readOperation_);
    ASSERT_TRUE(cbHandler->Wait());
    pthread_join(readThread, NULL);

    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation_, global::clientEndpointName );
    EXPECT_TRUE(readResponse != NULL);

    switch (data.Type)
    {
        case AwaResourceType_String:
            {
                const char * value;
                EXPECT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsCStringPointer(readResponse, path, (const char **)&value));
                ASSERT_STREQ((char*) data.Value, (char*) value);
                break;
            }
        case AwaResourceType_Integer:
            {
                AwaInteger * value;
                EXPECT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsIntegerPointer(readResponse, path, (const AwaInteger **)&value));
                ASSERT_EQ(*static_cast<const AwaInteger *>(data.Value), *static_cast<AwaInteger *>(value));
                break;
            }
        case AwaResourceType_Float:
            {
                AwaFloat * value;
                EXPECT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsFloatPointer(readResponse, path, (const AwaFloat **)&value));
                ASSERT_EQ(*static_cast<const AwaFloat *>(data.Value), *static_cast<AwaFloat *>(value));
                break;
            }
        case AwaResourceType_Boolean:
            {
                AwaBoolean * value;
                EXPECT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsBooleanPointer(readResponse, path, (const AwaBoolean **)&value));
                ASSERT_EQ(*static_cast<const AwaBoolean *>(data.Value), *static_cast<AwaBoolean *>(value));
                break;
            }
        case AwaResourceType_Opaque:
            {
                AwaOpaque * value;
                EXPECT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsOpaquePointer(readResponse, path, (const AwaOpaque **)&value));
                ASSERT_EQ(data.ValueSize, static_cast<int>(value->Size));
                ASSERT_TRUE(memcmp(value->Data, data.Value, data.ValueSize) == 0);
                break;
            }
        case AwaResourceType_Time:
            {
                AwaTime * value;
                EXPECT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsTimePointer(readResponse, path, (const AwaTime **)&value));
                ASSERT_EQ(*static_cast<const AwaTime *>(data.Value), *static_cast<AwaTime *>(value));
                break;
            }
        case AwaResourceType_ObjectLink:
            {
                AwaObjectLink * expectedObjectLink = (AwaObjectLink *)data.Value;
                const AwaObjectLink * receivedObjectLinkPointer;
                EXPECT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsObjectLinkPointer(readResponse, path, (const AwaObjectLink **)&receivedObjectLinkPointer));
                EXPECT_EQ(expectedObjectLink->ObjectID, receivedObjectLinkPointer->ObjectID);
                EXPECT_EQ(expectedObjectLink->ObjectInstanceID, receivedObjectLinkPointer->ObjectInstanceID);
                break;
            }
    default:
        EXPECT_TRUE(false);
        break;
    }

}

INSTANTIATE_TEST_CASE_P(
        TestStaticClientHandlerWriteReadValueDeprecated,
        TestStaticClientHandlerWriteReadValueDeprecated,
        ::testing::Values(
        TestWriteReadStaticResourceDeprecated {TestWriteValueStaticClient_WriteHandler, TestWriteValueStaticClient_ReadHandler, writeDetailDeprecated::TEST_OBJECT_NON_ARRAY_TYPES, 0, writeDetailDeprecated::TEST_RESOURCE_STRING,     writeDetailDeprecated::dummyString1,      1, static_cast<int>(strlen(writeDetailDeprecated::dummyString1)),     AwaResourceType_String,     true, false},
        TestWriteReadStaticResourceDeprecated {TestWriteValueStaticClient_WriteHandler, TestWriteValueStaticClient_ReadHandler, writeDetailDeprecated::TEST_OBJECT_NON_ARRAY_TYPES, 0, writeDetailDeprecated::TEST_RESOURCE_INTEGER,    &writeDetailDeprecated::dummyInteger1,    1, static_cast<int>(sizeof(writeDetailDeprecated::dummyInteger1)),    AwaResourceType_Integer,    true, false},
        TestWriteReadStaticResourceDeprecated {TestWriteValueStaticClient_WriteHandler, TestWriteValueStaticClient_ReadHandler, writeDetailDeprecated::TEST_OBJECT_NON_ARRAY_TYPES, 0, writeDetailDeprecated::TEST_RESOURCE_FLOAT,      &writeDetailDeprecated::dummyFloat1,      1, static_cast<int>(sizeof(writeDetailDeprecated::dummyFloat1)),      AwaResourceType_Float,      true, false},
        TestWriteReadStaticResourceDeprecated {TestWriteValueStaticClient_WriteHandler, TestWriteValueStaticClient_ReadHandler, writeDetailDeprecated::TEST_OBJECT_NON_ARRAY_TYPES, 0, writeDetailDeprecated::TEST_RESOURCE_BOOLEAN,    &writeDetailDeprecated::dummyBoolean1,    1, static_cast<int>(sizeof(writeDetailDeprecated::dummyBoolean1)),    AwaResourceType_Boolean,    true, false},
        TestWriteReadStaticResourceDeprecated {TestWriteValueStaticClient_WriteHandler, TestWriteValueStaticClient_ReadHandler, writeDetailDeprecated::TEST_OBJECT_NON_ARRAY_TYPES, 0, writeDetailDeprecated::TEST_RESOURCE_OPAQUE,     writeDetailDeprecated::dummyOpaqueData,   1, static_cast<int>(sizeof(writeDetailDeprecated::dummyOpaqueData)),  AwaResourceType_Opaque,     true, false},
        TestWriteReadStaticResourceDeprecated {TestWriteValueStaticClient_WriteHandler, TestWriteValueStaticClient_ReadHandler, writeDetailDeprecated::TEST_OBJECT_NON_ARRAY_TYPES, 0, writeDetailDeprecated::TEST_RESOURCE_TIME,       &writeDetailDeprecated::dummyTime1,       1, static_cast<int>(sizeof(writeDetailDeprecated::dummyTime1)),       AwaResourceType_Time,       true, false},
        TestWriteReadStaticResourceDeprecated {TestWriteValueStaticClient_WriteHandler, TestWriteValueStaticClient_ReadHandler, writeDetailDeprecated::TEST_OBJECT_NON_ARRAY_TYPES, 0, writeDetailDeprecated::TEST_RESOURCE_OBJECTLINK, &writeDetailDeprecated::dummyObjectLink1, 1, static_cast<int>(sizeof(writeDetailDeprecated::dummyObjectLink1)), AwaResourceType_ObjectLink, true, false}
        ));
} // namespace Awa
