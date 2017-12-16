// © 2017 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License
package com.ibm.icu.impl.number.parse;

import java.util.EnumMap;
import java.util.Map;

import com.ibm.icu.text.UnicodeSet;

/**
 * @author sffc
 *
 */
public class UnicodeSetStaticCache {
    public static enum Key {
        // Ignorables
        BIDI,
        WHITESPACE,
        DEFAULT_IGNORABLES,
        STRICT_IGNORABLES,

        // Separators
        COMMA,
        PERIOD,
        OTHER_GROUPING_SEPARATORS,
        COMMA_OR_OTHER,
        PERIOD_OR_OTHER,
        COMMA_OR_PERIOD_OR_OTHER,
        STRICT_COMMA,
        STRICT_PERIOD,
        STRICT_COMMA_OR_OTHER,
        STRICT_PERIOD_OR_OTHER,
        STRICT_COMMA_OR_PERIOD_OR_OTHER,

        // Symbols
        // TODO: NaN?
        MINUS_SIGN,
        PLUS_SIGN,
        PERCENT_SIGN,
        PERMILLE_SIGN,
        INFINITY,

        // Other
        DIGITS,
    };

    private static final Map<Key, UnicodeSet> unicodeSets = new EnumMap<Key, UnicodeSet>(Key.class);
    private static final Map<Key, UnicodeSet> leadCharsSets = new EnumMap<Key, UnicodeSet>(Key.class);

    public static UnicodeSet get(Key key) {
        return unicodeSets.get(key);
    }

    public static UnicodeSet getLeadChars(Key key) {
        return leadCharsSets.get(key);
    }

    public static Key chooseFrom(String str, Key key1) {
        return get(key1).contains(str) ? key1 : null;
    }

    public static Key chooseFrom(String str, Key key1, Key key2) {
        return get(key1).contains(str) ? key1 : chooseFrom(str, key2);
    }

    public static Key chooseFrom(String str, Key key1, Key key2, Key key3) {
        return get(key1).contains(str) ? key1 : chooseFrom(str, key2, key3);
    }

    public static Key unionOf(Key key1, Key key2) {
        // Make sure key1 < key2
        if (key2.ordinal() < key1.ordinal()) {
            Key temp = key1;
            key1 = key2;
            key2 = temp;
        }

        if (key1 == Key.COMMA && key2 == Key.PERIOD_OR_OTHER) {
            // 1.234,567
            return Key.COMMA_OR_PERIOD_OR_OTHER;

        } else if (key1 == Key.COMMA && key2 == Key.OTHER_GROUPING_SEPARATORS) {
            // 1'234,567
            return Key.COMMA_OR_OTHER;

        } else if (key1 == Key.PERIOD && key2 == Key.COMMA_OR_OTHER) {
            // 1,234.567
            return Key.COMMA_OR_PERIOD_OR_OTHER;

        } else if (key1 == Key.PERIOD && key2 == Key.OTHER_GROUPING_SEPARATORS) {
            // 1'234.567
            return Key.PERIOD_OR_OTHER;

        } else if (key1 == Key.STRICT_COMMA && key2 == Key.STRICT_PERIOD_OR_OTHER) {
            // Strict 1.234,567
            return Key.STRICT_COMMA_OR_PERIOD_OR_OTHER;

        } else if (key1 == Key.STRICT_COMMA && key2 == Key.OTHER_GROUPING_SEPARATORS) {
            // Strict 1'234,567
            return Key.STRICT_COMMA_OR_OTHER;

        } else if (key1 == Key.STRICT_PERIOD && key2 == Key.STRICT_COMMA_OR_OTHER) {
            // Strict 1,234.567
            return Key.STRICT_COMMA_OR_PERIOD_OR_OTHER;

        } else if (key1 == Key.STRICT_PERIOD && key2 == Key.OTHER_GROUPING_SEPARATORS) {
            // Strict 1'234.567
            return Key.STRICT_PERIOD_OR_OTHER;

        }

        return null;
    }

    private static UnicodeSet computeUnion(Key k1, Key k2) {
        return new UnicodeSet().addAll(get(k1)).addAll(get(k2)).freeze();
    }

    private static UnicodeSet computeUnion(Key k1, Key k2, Key k3) {
        return new UnicodeSet().addAll(get(k1)).addAll(get(k2)).addAll(get(k3)).freeze();
    }

    static {
        // BiDi characters are skipped over and ignored at any point in the string, even in strict mode.
        unicodeSets.put(Key.BIDI, new UnicodeSet("[[\\u200E\\u200F\\u061C]]").freeze());

        // This set was decided after discussion with icu-design@. See ticket #13309.
        // Zs+TAB is "horizontal whitespace" according to UTS #18 (blank property).
        unicodeSets.put(Key.WHITESPACE, new UnicodeSet("[[:Zs:][\\u0009]]").freeze());

        unicodeSets.put(Key.DEFAULT_IGNORABLES, computeUnion(Key.BIDI, Key.WHITESPACE));
        unicodeSets.put(Key.STRICT_IGNORABLES, get(Key.BIDI));

        // TODO: Re-generate these sets from the UCD. They probably haven't been updated in a while.
        unicodeSets.put(Key.COMMA,
                new UnicodeSet("[,\\u060C\\u066B\\u3001\\uFE10\\uFE11\\uFE50\\uFE51\\uFF0C\\uFF64]").freeze());
        unicodeSets.put(Key.STRICT_COMMA, new UnicodeSet("[,\\u066B\\uFE10\\uFE50\\uFF0C]").freeze());
        unicodeSets.put(Key.PERIOD, new UnicodeSet("[.\\u2024\\u3002\\uFE12\\uFE52\\uFF0E\\uFF61]").freeze());
        unicodeSets.put(Key.STRICT_PERIOD, new UnicodeSet("[.\\u2024\\uFE52\\uFF0E\\uFF61]").freeze());
        unicodeSets.put(Key.OTHER_GROUPING_SEPARATORS,
                new UnicodeSet("[\\ '\\u00A0\\u066C\\u2000-\\u200A\\u2018\\u2019\\u202F\\u205F\\u3000\\uFF07]")
                        .freeze());

        unicodeSets.put(Key.COMMA_OR_OTHER, computeUnion(Key.COMMA, Key.OTHER_GROUPING_SEPARATORS));
        unicodeSets.put(Key.PERIOD_OR_OTHER, computeUnion(Key.PERIOD, Key.OTHER_GROUPING_SEPARATORS));
        unicodeSets.put(Key.COMMA_OR_PERIOD_OR_OTHER,
                computeUnion(Key.COMMA, Key.PERIOD, Key.OTHER_GROUPING_SEPARATORS));
        unicodeSets.put(Key.STRICT_COMMA_OR_OTHER, computeUnion(Key.STRICT_COMMA, Key.OTHER_GROUPING_SEPARATORS));
        unicodeSets.put(Key.STRICT_PERIOD_OR_OTHER, computeUnion(Key.STRICT_PERIOD, Key.OTHER_GROUPING_SEPARATORS));
        unicodeSets.put(Key.STRICT_COMMA_OR_PERIOD_OR_OTHER,
                computeUnion(Key.STRICT_COMMA, Key.STRICT_PERIOD, Key.OTHER_GROUPING_SEPARATORS));

        unicodeSets.put(Key.MINUS_SIGN,
                new UnicodeSet(0x002D,
                        0x002D,
                        0x207B,
                        0x207B,
                        0x208B,
                        0x208B,
                        0x2212,
                        0x2212,
                        0x2796,
                        0x2796,
                        0xFE63,
                        0xFE63,
                        0xFF0D,
                        0xFF0D).freeze());
        unicodeSets.put(Key.PLUS_SIGN,
                new UnicodeSet(0x002B,
                        0x002B,
                        0x207A,
                        0x207A,
                        0x208A,
                        0x208A,
                        0x2795,
                        0x2795,
                        0xFB29,
                        0xFB29,
                        0xFE62,
                        0xFE62,
                        0xFF0B,
                        0xFF0B).freeze());

        // TODO: Fill in the next three sets.
        unicodeSets.put(Key.PERCENT_SIGN, new UnicodeSet("[%٪]").freeze());
        unicodeSets.put(Key.PERMILLE_SIGN, new UnicodeSet("[‰؉]").freeze());
        unicodeSets.put(Key.INFINITY, new UnicodeSet("[∞]").freeze());

        unicodeSets.put(Key.DIGITS, new UnicodeSet("[:digit:]").freeze());

        for (Key key : Key.values()) {
            UnicodeSet leadChars = new UnicodeSet();
            ParsingUtils.putLeadSurrogates(get(key), leadChars);
            leadCharsSets.put(key, leadChars.freeze());
        }
    }
}
