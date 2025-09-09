// Simple test to verify unit testing framework works
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSimpleUnitTest, 
    "EasyOnline.DataTransfer.Unit.Simple", 
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSimpleUnitTest::RunTest(const FString& Parameters)
{
    TestTrue(TEXT("Simple test should pass"), true);
    TestFalse(TEXT("False should be false"), false);
    TestEqual(TEXT("1 should equal 1"), 1, 1);
    
    return true;
}