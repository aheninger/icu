// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
/********************************************************************
 * COPYRIGHT:
 * Copyright (c) 2004-2011, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/

//      Test parts of UVector and UStack

#include "intltest.h"

#include "uvectest.h"
#include "cstring.h"
#include "hash.h"
#include "uelement.h"
#include "uvector.h"

//---------------------------------------------------------------------------
//
//  Test class boilerplate
//
//---------------------------------------------------------------------------
UVectorTest::UVectorTest() 
{
}


UVectorTest::~UVectorTest()
{
}


void UVectorTest::runIndexedTest(int32_t index, UBool exec, const char* &name, char* par)
{
    (void)par;
    TESTCASE_AUTO_BEGIN;
    TESTCASE_AUTO(UVector_API);
    TESTCASE_AUTO(UStack_API);
    TESTCASE_AUTO(Hashtable_API);
    TESTCASE_AUTO(UVector_ErrorHandling);
    TESTCASE_AUTO_END;
}


static int8_t U_CALLCONV
UVectorTest_compareInt32(UElement key1, UElement key2) {
    if (key1.integer > key2.integer) {
        return 1;
    }
    else if (key1.integer < key2.integer) {
        return -1;
    }
    return 0;
}

U_CDECL_BEGIN
static int8_t U_CALLCONV
UVectorTest_compareCstrings(const UElement key1, const UElement key2) {
    return !strcmp((const char *)key1.pointer, (const char *)key2.pointer);
}
U_CDECL_END

//---------------------------------------------------------------------------
//
//      UVector_API      Check for basic functionality of UVector.
//
//---------------------------------------------------------------------------
void UVectorTest::UVector_API() {

    UErrorCode  status = U_ZERO_ERROR;
    UVector     *a;

    a = new UVector(status);
    assertSuccess(WHERE, status);
    delete a;

    status = U_ZERO_ERROR;
    a = new UVector(2000, status);
    assertSuccess(WHERE, status);
    delete a;

    status = U_ZERO_ERROR;
    a = new UVector(status);
    a->sortedInsert((int32_t)10, UVectorTest_compareInt32, status);
    a->sortedInsert((int32_t)20, UVectorTest_compareInt32, status);
    a->sortedInsert((int32_t)30, UVectorTest_compareInt32, status);
    a->sortedInsert((int32_t)15, UVectorTest_compareInt32, status);
    assertSuccess(WHERE, status);
    assertEquals(WHERE, 10, a->elementAti(0));
    assertEquals(WHERE, 15, a->elementAti(1));
    assertEquals(WHERE, 20, a->elementAti(2));
    assertEquals(WHERE, 30, a->elementAti(3));
    assertEquals(WHERE, -1, a->indexOf(3));
    assertEquals(WHERE,  1, a->indexOf(15));
    assertEquals(WHERE, -1, a->indexOf(15, 2));
    assertTrue(WHERE, a->contains(15));
    assertFalse(WHERE, a->contains(5));
    delete a;
}

void UVectorTest::UStack_API() {
    UErrorCode  status = U_ZERO_ERROR;
    UStack     *a;

    a = new UStack(status);
    assertSuccess(WHERE, status);
    delete a;

    status = U_ZERO_ERROR;
    a = new UStack(2000, status);
    assertSuccess(WHERE, status);
    delete a;

    status = U_ZERO_ERROR;
    a = new UStack(nullptr, nullptr, 2000, status);
    assertSuccess(WHERE, status);
    delete a;

    status = U_ZERO_ERROR;
    a = new UStack(nullptr, UVectorTest_compareCstrings, status);
    assertTrue(WHERE, a->empty());
    a->push((void*)"abc", status);
    assertFalse(WHERE, a->empty());
    a->push((void*)"bcde", status);
    a->push((void*)"cde", status);
    assertSuccess(WHERE, status);
    assertEquals(WHERE, "cde", (const char *)a->peek());
    assertEquals(WHERE, 1, a->search((void*)"cde"));
    assertEquals(WHERE, 2, a->search((void*)"bcde"));
    assertEquals(WHERE, 3, a->search((void*)"abc"));
    assertEquals(WHERE, "abc", (const char *)a->firstElement());
    assertEquals(WHERE, "cde", (const char *)a->lastElement());
    assertEquals(WHERE, "cde", (const char *)a->pop());
    assertEquals(WHERE, "bcde", (const char *)a->pop());
    assertEquals(WHERE, "abc", (const char *)a->pop());
    delete a;
}

U_CDECL_BEGIN
static UBool U_CALLCONV neverTRUE(const UElement /*key1*/, const UElement /*key2*/) {
    return FALSE;
}

U_CDECL_END

void UVectorTest::Hashtable_API() {
    UErrorCode status = U_ZERO_ERROR;
    Hashtable *a = new Hashtable(status);
    assertEquals(WHERE, 0, a->puti(u"a", 1, status));
    assertTrue(WHERE, a->find(u"a") != nullptr);
    assertTrue(WHERE, a->find(u"b") == nullptr);
    assertEquals(WHERE, 0, a->puti(u"b", 2, status));
    assertTrue(WHERE, a->find(u"b") != nullptr);
    assertEquals(WHERE, 1, a->removei(u"a"));
    assertTrue(WHERE, a->find(u"a") == nullptr);

    /* verify that setValueComparator works */
    Hashtable b(status);
    assertFalse(WHERE, a->equals(b));
    assertEquals(WHERE, 0, b.puti("b", 2, status));
    assertFalse(WHERE, a->equals(b)); // Without a value comparator, this will be FALSE by default.
    b.setValueComparator(uhash_compareLong);
    assertFalse(WHERE, a->equals(b));
    a->setValueComparator(uhash_compareLong);
    assertTrue(WHERE, a->equals(b));
    assertTrue(WHERE, a->equals(*a)); // This better be reflexive.

    /* verify that setKeyComparator works */
    assertEquals(WHERE, 0, a->puti(u"a", 1, status));
    assertTrue(WHERE, a->find(u"a") != nullptr);
    a->setKeyComparator(neverTRUE);
    assertTrue(WHERE, a->find(u"a") == nullptr);

    delete a;
}

/*
 * UVector_ErrorHandling. Issue ICU-21315
 * Functions with an incoming UErrorCode error should take no action.
 */
void UVectorTest::UVector_ErrorHandling() {
    {
        UErrorCode status = U_ZERO_ERROR;
        UVector v(status);
        assertSuccess(WHERE, status);

        UnicodeString s = u"Hello, World.";
        v.addElement(&s, status);
        assertSuccess(WHERE, status);
        assertEquals(WHERE, 1, v.size());

        status = U_INVALID_FORMAT_ERROR;   // Arbitrary incoming error.
        v.addElement(&s, status);
        assertEquals(WHERE, U_INVALID_FORMAT_ERROR, status);  // error unchanged.
        assertEquals(WHERE, 1, v.size());    // addElement did not add.
    }

    {
        // For UVectors with  a deleter function, adding an element adopts
        // the object being added. In case of error, the incoming object
        // is immediately deleted, otherwise it would leak.
        UErrorCode status = U_ZERO_ERROR;
        UVector v(uprv_deleteUObject, nullptr, status);
        assertSuccess(WHERE, status);
        v.addElement(new UnicodeString(u"Hello, World."), status);
        assertSuccess(WHERE, status);
        assertEquals(WHERE, 1, v.size());

        status = U_INVALID_FORMAT_ERROR;
        v.addElement(new UnicodeString(u"Hello, World."), status);
        assertEquals(WHERE, U_INVALID_FORMAT_ERROR, status);  // error unchanged.
        assertEquals(WHERE, 1, v.size());    // addElement did not add.

    }
}
