/* date = March 26th 2022 5:17 pm */

#ifndef BASE_STRING_H
#define BASE_STRING_H

typedef struct String8 String;

typedef struct String8 String8;
struct String8
{
    union
    {
        void *data;
        u8 *str;
        char *cstr;
    };
    union
    {
        u64 size;
        u64 len;
    };
    
};

typedef struct String16 String16;
struct String16
{
    U16 *str;
    U64 size;
};

typedef struct String32 String32;
struct String32
{
    U32 *str;
    U64 size;
};

typedef struct String8Node String8Node;
struct String8Node
{
    String8Node *next;
    String8 string;
};

typedef struct String8List String8List;
struct String8List
{
    String8Node *first;
    String8Node *last;
    U64 node_count;
    U64 total_size;
};

typedef struct StringJoin StringJoin;
struct StringJoin
{
    String8 pre;
    String8 sep;
    String8 post;
};

typedef U32 MatchFlags;
enum
{
    MatchFlag_CaseInsensitive  = (1<<0),
    MatchFlag_RightSideSloppy  = (1<<1),
    MatchFlag_SlashInsensitive = (1<<2),
    MatchFlag_FindLast         = (1<<3),
};

typedef struct DecodedCodepoint DecodedCodepoint;
struct DecodedCodepoint
{
    U32 codepoint;
    U32 advance;
};

typedef enum IdentifierStyle
{
    IdentifierStyle_UpperCamelCase,
    IdentifierStyle_LowerCamelCase,
    IdentifierStyle_UpperCase,
    IdentifierStyle_LowerCase,
}
IdentifierStyle;

////////////////////////////////
//~ rjf: Char Functions

internal B32 CharIsAlpha(U8 c);
internal B32 CharIsAlphaUpper(U8 c);
internal B32 CharIsAlphaLower(U8 c);
internal B32 CharIsDigit(U8 c);
internal B32 CharIsSymbol(U8 c);
internal B32 CharIsSpace(U8 c);
internal U8  CharToUpper(U8 c);
internal U8  CharToLower(U8 c);
internal U8  CharToForwardSlash(U8 c);

////////////////////////////////
//~ rjf: String Functions

//- rjf: Helpers
internal U64 CalculateCStringLength(char *cstr);

//- rjf: Basic Constructors
internal String8 Str8(U8 *str, U64 size);
#define Str8C(cstring) Str8((U8 *)(cstring), CalculateCStringLength(cstring))
#define Str8Lit(s) Str8((U8 *)(s), sizeof(s)-1)
#define Str8LitComp(s) {(U8 *)(s), sizeof(s)-1}
internal String8 Str8Range(U8 *first, U8 *one_past_last);
internal String16 Str16(U16 *str, U64 size);
internal String16 Str16C(U16 *ptr);
#define Str8Struct(ptr) Str8((U8 *)(ptr), sizeof(*(ptr)))
internal String8Node Str8Node(String8 str);

//- rjf: Substrings
internal String8 Substr8(String8 str, U64 min, U64 max);
internal String8 Str8Skip(String8 str, U64 min);
internal String8 Str8Chop(String8 str, U64 nmax);
internal String8 Prefix8(String8 str, U64 size);
internal String8 Suffix8(String8 str, U64 size);

//- rjf: Matching
internal B32 Str8Match(String8 a, String8 b, MatchFlags flags);
internal U64 FindSubstr8(String8 haystack, String8 needle, U64 start_pos, MatchFlags flags);

//- rjf: Allocation
internal String8 PushStr8Copy(M_Arena *arena, String8 string);
internal String8 PushStr8FV(M_Arena *arena, char *fmt, va_list args);
internal String8 PushStr8F(M_Arena *arena, char *fmt, ...);
internal String8 PushReservedStr8(M_Arena *arena, u64 size);
//- rjf: Use In Format Strings
#define Str8VArg(s) (int)(s).size, (s).cstr

//- rjf: String Lists
internal void Str8ListPushNode(String8List *list, String8Node *n);
internal void Str8ListPush(M_Arena *arena, String8List *list, String8 str);
internal void Str8ListConcat(String8List *list, String8List *to_push);
internal String8List StrSplit8(M_Arena *arena, String8 string, int split_count, String8 *splits);
internal String8 Str8ListJoin(M_Arena *arena, String8List list, StringJoin *optional_params);

//- rjf: String Re-Styling
internal String8 Str8Stylize(M_Arena *arena, String8 string, IdentifierStyle style, String8 separator);

////////////////////////////////
//~ rjf: Unicode Conversions

internal DecodedCodepoint DecodeCodepointFromUtf8(U8 *str, U64 max);
internal DecodedCodepoint DecodeCodepointFromUtf16(U16 *str, U64 max);
internal U32              Utf8FromCodepoint(U8 *out, U32 codepoint);
internal U32              Utf16FromCodepoint(U16 *out, U32 codepoint);
internal String8          Str8From16(M_Arena *arena, String16 str);
internal String16         Str16From8(M_Arena *arena, String8 str);
internal String8          Str8From32(M_Arena *arena, String32 str);
internal String32         Str32From8(M_Arena *arena, String8 str);

////////////////////////////////
//~ rjf: Skip/Chop Helpers

internal String8 Str8ChopLastPeriod(String8 str);
internal String8 Str8SkipLastSlash(String8 str);
internal String8 Str8SkipLastPeriod(String8 str);
internal String8 Str8ChopLastSlash(String8 str);

////////////////////////////////
//~ rjf: Numeric Conversions

internal U64 U64FromStr8(String8 str, U32 radix);
internal S64 CStyleIntFromStr8(String8 str);
internal F64 F64FromStr8(String8 str);
internal String8 CStyleHexStringFromU64(M_Arena *arena, U64 x, B32 caps);

#endif //BASE_STRING_H
