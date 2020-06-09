// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
//
//  rbbisetb.h
/*
**********************************************************************
*   Copyright (c) 2001-2005, International Business Machines
*   Corporation and others.  All Rights Reserved.
**********************************************************************
*/

#ifndef RBBISETB_H
#define RBBISETB_H

#include "unicode/utypes.h"

#if !UCONFIG_NO_BREAK_ITERATION

#include "unicode/ucptrie.h"
#include "unicode/umutablecptrie.h"
#include "unicode/uobject.h"
#include "rbbirb.h"
#include "uvector.h"

U_NAMESPACE_BEGIN

//
//  RBBISetBuilder   Derives the character categories used by the runtime RBBI engine
//                   from the Unicode Sets appearing in the source  RBBI rules, and
//                   creates the TRIE table used to map from Unicode to the
//                   character categories.
//


//
//  RangeDescriptor
//
//     Each of the non-overlapping character ranges gets one of these descriptors.
//     All of them are strung together in a linked list, which is kept in order
//     (by character)
//
class RangeDescriptor : public UMemory {
public:
    UChar32            fStartChar {};      // Start of range, unicode 32 bit value.
    UChar32            fEndChar {};        // End of range, unicode 32 bit value.
    bool               fIncludesDict {};   // True if the range includes $dictionary
    UVector           *fIncludesSets {};   // vector of the the original
                                           //   Unicode sets that include this range.
                                           //    (Contains ptrs to uset nodes)
    RangeDescriptor   *fNext {};           // Next RangeDescriptor in the linked list.

    RangeDescriptor(UErrorCode &status);
    RangeDescriptor(const RangeDescriptor &other, UErrorCode &status);
    ~RangeDescriptor();
    void split(UChar32 where, UErrorCode &status);   // Spit this range in two at "where", with
                                        //   where appearing in the second (higher) part.
    bool isDictionaryRange();           // Check whether this range appears as part of
                                        //   the Unicode set named "dictionary"

    RangeDescriptor(const RangeDescriptor &other) = delete; // forbid default copying of this class
    RangeDescriptor &operator=(const RangeDescriptor &other) = delete; // forbid assigning of this class
};


class RangeGroup : public UObject {
public:
    UVector             fRanges;         // UVector<RangeDescriptor *>
    UVector            *fIncludesSets;   // Not owned. Reference to the sets from
                                         // the first range added to the group.

    // A range group's number is the Trie value for characters in that group,
    // which is also the state table column index. But note that later state table
    // optimizations may change the originally assigned numbers.
    int32_t             fNum;

    RangeGroup(RangeDescriptor *rd, UErrorCode &fStatus);

};


//
//  RBBISetBuilder   Handles processing of Unicode Sets from RBBI rules.
//
//      Starting with the rules parse tree from the scanner,
//
//                   -  Enumerate the set of UnicodeSets that are referenced
//                      by the RBBI rules.
//                   -  compute a derived set of non-overlapping UnicodeSets
//                      that will correspond to columns in the state table for
//                      the RBBI execution engine.
//                   -  construct the trie table that maps input characters
//                      to set numbers in the non-overlapping set of sets.
//


class RBBISetBuilder : public UMemory {
public:
    RBBISetBuilder(RBBIRuleBuilder *rb);
    ~RBBISetBuilder();

    void     buildRanges();
    void     buildTrie();
    void     addValToSets(UVector *sets,      uint32_t val);
    void     addValToSet (RBBINode *usetNode, uint32_t val);
    int32_t  getNumCharCategories() const;   // CharCategories are the same as input symbol set to the
                                             //    runtime state machine, which are the same as
                                             //    columns in the DFA state table
    int32_t  getTrieSize() /*const*/;        // Size in bytes of the serialized Trie.
    void     serializeTrie(uint8_t *where);  // write out the serialized Trie.
    UChar32  getFirstChar(int32_t  val) const;
    RangeGroup *groupForCategory(int32_t cat) const;
    UBool    sawBOF() const;                 // Indicate whether any references to the {bof} pseudo
                                             //   character were encountered.
    /**
     * Merge two character categories that have been identified as having equivalent behavior.
     * The ranges belonging to the second category (table column) will be added to the first.
     * @param categories the pair of categories to be merged.
     */
    void     mergeCategories(IntPair categories);

    int32_t  numberForRange(RangeDescriptor *r) const;
#ifdef RBBI_DEBUG
    void     printSets();
    void     printRanges();
    void     printRangeGroups();
#else
    #define printSets()
    #define printRanges()
    #define printRangeGroups()
#endif

private:
    RBBIRuleBuilder       *fRB;             // The RBBI Rule Compiler that owns us.
    UErrorCode            *fStatus;

    RangeDescriptor       *fRangeList;      // Head of the linked list of RangeDescriptors
    UVector               *fRangeGroups;    // UVector<RangeGroup>

    UMutableCPTrie        *fMutableTrie;    // The mapping TRIE that is the end result of processing
    UCPTrie               *fTrie;           //  the Unicode Sets.
    uint32_t               fTrieSize;

    // Number of range groups, which are groups of ranges that are in the same original UnicodeSets.
    int32_t               fGroupCount;

    // The number of the first dictionary char category.
    // If there are no Dictionary categories, set to the last category + 1.
    int32_t               fDictCategoriesStart;

    UBool                 fSawBOF;

    RBBISetBuilder(const RBBISetBuilder &other); // forbid copying of this class
    RBBISetBuilder &operator=(const RBBISetBuilder &other); // forbid copying of this class
};



U_NAMESPACE_END

#endif /* #if !UCONFIG_NO_BREAK_ITERATION */

#endif
