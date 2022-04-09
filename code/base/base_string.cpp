#define STB_SPRINTF_IMPLEMENTATION
#include "third_party/stb_sprintf.h"

////////////////////////////////
//~ rjf: Char Functions

internal B32
CharIsAlpha(U8 c)
{
    return CharIsAlphaUpper(c) || CharIsAlphaLower(c);
}

internal B32
CharIsAlphaUpper(U8 c)
{
    return c >= 'A' && c <= 'Z';
}

internal B32
CharIsAlphaLower(U8 c)
{
    return c >= 'a' && c <= 'z';
}

internal B32
CharIsDigit(U8 c)
{
    return (c >= '0' && c <= '9');
}

internal B32
CharIsSymbol(U8 c)
{
    return (c == '~' || c == '!'  || c == '$' || c == '%' || c == '^' ||
            c == '&' || c == '*'  || c == '-' || c == '=' || c == '+' ||
            c == '<' || c == '.'  || c == '>' || c == '/' || c == '?' ||
            c == '|' || c == '\\' || c == '{' || c == '}' || c == '(' ||
            c == ')' || c == '\\' || c == '[' || c == ']' || c == '#' ||
            c == ',' || c == ';'  || c == ':' || c == '@');
}

internal B32
CharIsSpace(U8 c)
{
    return c == ' ' || c == '\r' || c == '\t' || c == '\f' || c == '\v';
}

internal U8
CharToUpper(U8 c)
{
    return (c >= 'a' && c <= 'z') ? ('A' + (c - 'a')) : c;
}

internal U8
CharToLower(U8 c)
{
    return (c >= 'A' && c <= 'Z') ? ('a' + (c - 'A')) : c;
}

internal U8
CharToForwardSlash(U8 c)
{
    return (c == '\\' ? '/' : c);
}

////////////////////////////////
//~ rjf: String Functions

//- rjf: Helpers

internal U64
CalculateCStringLength(char *cstr)
{
    U64 length = 0;
    for(;cstr[length]; length += 1);
    return length;
}

//- rjf: Basic Constructors

internal String8
Str8(U8 *str, U64 size)
{
    String8 string;
    string.str = str;
    string.size = size;
    return string;
}

internal String8
Str8Range(U8 *first, U8 *one_past_last)
{
    String8 string;
    string.str = first;
    string.size = (U64)(one_past_last - first);
    return string;
}

internal String16
Str16(U16 *str, U64 size)
{
    String16 result;
    result.str = str;
    result.size = size;
    return result;
}

internal String16
Str16C(U16 *ptr)
{
    U16 *p = ptr;
    for (;*p; p += 1);
    String16 result = Str16(ptr, p - ptr);
    return(result);
}

internal String8Node 
Str8Node(String8 str)
{
    String8Node result;
    
    result.string = str;
    result.next   = 0;
    
    return result;
}

//- rjf: Substrings

internal String8
Substr8(String8 str, U64 min, U64 max)
{
    if(max > str.size)
    {
        max = str.size;
    }
    if(min > str.size)
    {
        min = str.size;
    }
    if(min > max)
    {
        U64 swap = min;
        min = max;
        max = swap;
    }
    str.size = max - min;
    str.str += min;
    return str;
}

internal String8
Str8Skip(String8 str, U64 min)
{
    return Substr8(str, min, str.size);
}

internal String8
Str8Chop(String8 str, U64 nmax)
{
    return Substr8(str, 0, str.size-nmax);
}

internal String8
Prefix8(String8 str, U64 size)
{
    return Substr8(str, 0, size);
}

internal String8
Suffix8(String8 str, U64 size)
{
    return Substr8(str, str.size-size, str.size);
}

//- rjf: Matching

internal B32
Str8Match(String8 a, String8 b, MatchFlags flags)
{
    B32 result = 0;
    if(a.size == b.size || flags & MatchFlag_RightSideSloppy)
    {
        result = 1;
        for(U64 i = 0; i < a.size; i += 1)
        {
            B32 match = (a.str[i] == b.str[i]);
            if(flags & MatchFlag_CaseInsensitive)
            {
                match |= (CharToLower(a.str[i]) == CharToLower(b.str[i]));
            }
            if(flags & MatchFlag_SlashInsensitive)
            {
                match |= (CharToForwardSlash(a.str[i]) == CharToForwardSlash(b.str[i]));
            }
            if(match == 0)
            {
                result = 0;
                break;
            }
        }
    }
    return result;
}

internal U64
FindSubstr8(String8 haystack, String8 needle, U64 start_pos, MatchFlags flags)
{
    B32 found = 0;
    U64 found_idx = haystack.size;
    for(U64 i = start_pos; i < haystack.size; i += 1)
    {
        if(i + needle.size <= haystack.size)
        {
            String8 substr = Substr8(haystack, i, i+needle.size);
            if(Str8Match(substr, needle, flags))
            {
                found_idx = i;
                found = 1;
                if(!(flags & MatchFlag_FindLast))
                {
                    break;
                }
            }
        }
    }
    return found_idx;
}

//- rjf: Allocation

internal String8
PushStr8Copy(M_Arena *arena, String8 string)
{
    String8 res;
    res.size = string.size;
    res.str = PushArray(arena, U8, string.size + 1);
    MemoryCopy(res.str, string.str, string.size);
    res.str[string.size] = 0;
    return res;
}

internal String8
PushStr8FV(M_Arena *arena, char *fmt, va_list args)
{
    String8 result = {0};
    va_list args2;
    va_copy(args2, args);
    u32 needed_bytes = stbsp_vsnprintf(0, 0, fmt, args)+1;
    result.str = PushArray(arena, U8, needed_bytes);
    result.size = needed_bytes - 1;
    stbsp_vsnprintf((char*)result.str, needed_bytes, fmt, args2);
    return result;
}

internal String8
PushStr8F(M_Arena *arena, char *fmt, ...)
{
    String8 result = {0};
    va_list args;
    va_start(args, fmt);
    result = PushStr8FV(arena, fmt, args);
    va_end(args);
    return result;
}

internal String8 
PushReservedStr8(M_Arena *arena, u64 size)
{
    String8 result;
    result.str  = PushArray(arena, u8, size + 1);
    result.size = size;
    return result;
}
//- rjf: String Lists

internal void
Str8ListPushNode(String8List *list, String8Node *node)
{
    if (list->first == 0)
    {
        // NOTE(fakhri): empty list
        list->first = list->last = node;
        node->next = 0;
    }
    else
    {
        String8Node *last = list->last;
        last->next = node;
        node->next = 0;
        list->last = node;
    }
    list->node_count += 1;
    list->total_size += node->string.size;
}

internal void
Str8ListPush(M_Arena *arena, String8List *list, String8 str)
{
    String8Node *n = PushArrayZero(arena, String8Node, 1);
    n->string = str;
    Str8ListPushNode(list, n);
}

internal void
Str8ListConcat(String8List *list, String8List *to_push)
{
    if(to_push->first)
    {
        list->node_count += to_push->node_count;
        list->total_size += to_push->total_size;
        if(list->last == 0)
        {
            *list = *to_push;
        }
        else
        {
            list->last->next = to_push->first;
            list->last = to_push->last;
        }
    }
    MemoryZero(to_push, sizeof(*to_push));
}

internal String8List
StrSplit8(M_Arena *arena, String8 string, int split_count, String8 *splits)
{
    String8List list = {0};
    
    U64 split_start = 0;
    for(U64 i = 0; i < string.size; i += 1)
    {
        B32 was_split = 0;
        for(int split_idx = 0; split_idx < split_count; split_idx += 1)
        {
            B32 match = 0;
            if(i + splits[split_idx].size <= string.size)
            {
                match = 1;
                for(U64 split_i = 0; split_i < splits[split_idx].size && i + split_i < string.size; split_i += 1)
                {
                    if(splits[split_idx].str[split_i] != string.str[i + split_i])
                    {
                        match = 0;
                        break;
                    }
                }
            }
            if(match)
            {
                String8 split_string = Str8(string.str + split_start, i - split_start);
                Str8ListPush(arena, &list, split_string);
                split_start = i + splits[split_idx].size;
                i += splits[split_idx].size - 1;
                was_split = 1;
                break;
            }
        }
        
        if(was_split == 0 && i == string.size - 1)
        {
            String8 split_string = Str8(string.str + split_start, i+1 - split_start);
            Str8ListPush(arena, &list, split_string);
            break;
        }
    }
    
    return list;
}

internal String8
Str8ListJoin(M_Arena *arena, String8List list, StringJoin *optional_params)
{
    // rjf: setup join parameters
    StringJoin join = {0};
    if(optional_params != 0)
    {
        MemoryCopy(&join, optional_params, sizeof(join));
    }
    
    // rjf: calculate size & allocate
    U64 sep_count = 0;
    if(list.node_count > 1)
    {
        sep_count = list.node_count - 1;
    }
    String8 result = {0};
    result.size = (list.total_size + join.pre.size +
                   sep_count*join.sep.size + join.post.size);
    result.str = PushArray(arena, U8, result.size + 1);
    
    // rjf: fill
    U8 *ptr = result.str;
    MemoryCopy(ptr, join.pre.str, join.pre.size);
    ptr += join.pre.size;
    for(String8Node *node = list.first; node; node = node->next)
    {
        MemoryCopy(ptr, node->string.str, node->string.size);
        ptr += node->string.size;
        if (node != list.last){
            MemoryCopy(ptr, join.sep.str, join.sep.size);
            ptr += join.sep.size;
        }
    }
    MemoryCopy(ptr, join.pre.str, join.pre.size);
    ptr += join.pre.size;
    
    result.str[result.size] = 0;
    return result;
}

//- rjf: String Re-Styling

internal String8
Str8Stylize(M_Arena *arena, String8 string, IdentifierStyle style, String8 separator)
{
    String8 result = {0};
    
    String8List words = {0};
    
    B32 break_on_uppercase = 0;
    {
        break_on_uppercase = 1;
        for(U64 i = 0; i < string.size; i += 1)
        {
            if(!CharIsAlpha(string.str[i]) && !CharIsDigit(string.str[i]))
            {
                break_on_uppercase = 0;
                break;
            }
        }
    }
    
    B32 making_word = 0;
    String8 word = {0};
    
    for(U64 i = 0; i < string.size;)
    {
        if(making_word)
        {
            if((break_on_uppercase && CharIsAlphaUpper(string.str[i])) ||
               string.str[i] == '_' || CharIsSpace(string.str[i]) ||
               i == string.size - 1)
            {
                if(i == string.size - 1)
                {
                    word.size += 1;
                }
                making_word = 0;
                Str8ListPush(arena, &words, word);
            }
            else
            {
                word.size += 1;
                i += 1;
            }
        }
        else
        {
            if(CharIsAlpha(string.str[i]))
            {
                making_word = 1;
                word.str = string.str + i;
                word.size = 1;
            }
            i += 1;
        }
    }
    
    result.size = words.total_size;
    if(words.node_count > 1)
    {
        result.size += separator.size*(words.node_count-1);
    }
    result.str = PushArray(arena, U8, result.size);
    
    {
        U64 write_pos = 0;
        for(String8Node *node = words.first; node; node = node->next)
        {
            
            // NOTE(rjf): Write word string to result.
            {
                MemoryCopy(result.str + write_pos, node->string.str, node->string.size);
                
                // NOTE(rjf): Transform string based on word style.
                switch(style)
                {
                    case IdentifierStyle_UpperCamelCase:
                    {
                        result.str[write_pos] = CharToUpper(result.str[write_pos]);
                        for(U64 i = write_pos+1; i < write_pos + node->string.size; i += 1)
                        {
                            result.str[i] = CharToLower(result.str[i]);
                        }
                    }break;
                    
                    case IdentifierStyle_LowerCamelCase:
                    {
                        result.str[write_pos] = node == words.first ? CharToLower(result.str[write_pos]) : CharToUpper(result.str[write_pos]);
                        for(U64 i = write_pos+1; i < write_pos + node->string.size; i += 1)
                        {
                            result.str[i] = CharToLower(result.str[i]);
                        }
                    }break;
                    
                    case IdentifierStyle_UpperCase:
                    {
                        for(U64 i = write_pos; i < write_pos + node->string.size; i += 1)
                        {
                            result.str[i] = CharToUpper(result.str[i]);
                        }
                    }break;
                    
                    case IdentifierStyle_LowerCase:
                    {
                        for(U64 i = write_pos; i < write_pos + node->string.size; i += 1)
                        {
                            result.str[i] = CharToLower(result.str[i]);
                        }
                    }break;
                    
                    default: break;
                }
                
                write_pos += node->string.size;
            }
            
            if(node->next)
            {
                MemoryCopy(result.str + write_pos, separator.str, separator.size);
                write_pos += separator.size;
            }
        }
    }
    
    return result;
}

////////////////////////////////
//~ rjf: Unicode Conversions

read_only global U8 utf8_class[32] =
{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5,
};

#define bitmask1 0x01
#define bitmask2 0x03
#define bitmask3 0x07
#define bitmask4 0x0F
#define bitmask5 0x1F
#define bitmask6 0x3F
#define bitmask7 0x7F
#define bitmask8 0xFF
#define bitmask9  0x01FF
#define bitmask10 0x03FF

internal DecodedCodepoint
DecodeCodepointFromUtf8(U8 *str, U64 max)
{
    DecodedCodepoint result = {~((U32)0), 1};
    U8 byte = str[0];
    U8 byte_class = utf8_class[byte >> 3];
    switch (byte_class)
    {
        case 1:
        {
            result.codepoint = byte;
        }break;
        
        case 2:
        {
            if (2 <= max)
            {
                U8 cont_byte = str[1];
                if (utf8_class[cont_byte >> 3] == 0)
                {
                    result.codepoint = (byte & bitmask5) << 6;
                    result.codepoint |=  (cont_byte & bitmask6);
                    result.advance = 2;
                }
            }
        }break;
        
        case 3:
        {
            if (3 <= max)
            {
                U8 cont_byte[2] = {str[1], str[2]};
                if (utf8_class[cont_byte[0] >> 3] == 0 &&
                    utf8_class[cont_byte[1] >> 3] == 0)
                {
                    result.codepoint = (byte & bitmask4) << 12;
                    result.codepoint |= ((cont_byte[0] & bitmask6) << 6);
                    result.codepoint |=  (cont_byte[1] & bitmask6);
                    result.advance = 3;
                }
            }
        }break;
        
        case 4:
        {
            if (4 <= max)
            {
                U8 cont_byte[3] = {str[1], str[2], str[3]};
                if (utf8_class[cont_byte[0] >> 3] == 0 &&
                    utf8_class[cont_byte[1] >> 3] == 0 &&
                    utf8_class[cont_byte[2] >> 3] == 0)
                {
                    result.codepoint = (byte & bitmask3) << 18;
                    result.codepoint |= ((cont_byte[0] & bitmask6) << 12);
                    result.codepoint |= ((cont_byte[1] & bitmask6) <<  6);
                    result.codepoint |=  (cont_byte[2] & bitmask6);
                    result.advance = 4;
                }
            }
        }break;
    }
    
    return result;
}

internal DecodedCodepoint
DecodeCodepointFromUtf16(U16 *out, U64 max)
{
    DecodedCodepoint result = {~((U32)0), 1};
    result.codepoint = out[0];
    result.advance = 1;
    if (1 < max && 0xD800 <= out[0] && out[0] < 0xDC00 && 0xDC00 <= out[1] && out[1] < 0xE000)
    {
        result.codepoint = ((out[0] - 0xD800) << 10) | (out[1] - 0xDC00);
        result.advance = 2;
    }
    return result;
}

internal U32             
Utf8FromCodepoint(U8 *out, U32 codepoint)
{
#define bit8 0x80
    U32 advance = 0;
    if (codepoint <= 0x7F)
    {
        out[0] = (U8)codepoint;
        advance = 1;
    }
    else if (codepoint <= 0x7FF)
    {
        out[0] = (bitmask2 << 6) | ((codepoint >> 6) & bitmask5);
        out[1] = bit8 | (codepoint & bitmask6);
        advance = 2;
    }
    else if (codepoint <= 0xFFFF)
    {
        out[0] = (bitmask3 << 5) | ((codepoint >> 12) & bitmask4);
        out[1] = bit8 | ((codepoint >> 6) & bitmask6);
        out[2] = bit8 | ( codepoint       & bitmask6);
        advance = 3;
    }
    else if (codepoint <= 0x10FFFF)
    {
        out[0] = (bitmask4 << 3) | ((codepoint >> 18) & bitmask3);
        out[1] = bit8 | ((codepoint >> 12) & bitmask6);
        out[2] = bit8 | ((codepoint >>  6) & bitmask6);
        out[3] = bit8 | ( codepoint        & bitmask6);
        advance = 4;
    }
    else
    {
        out[0] = '?';
        advance = 1;
    }
    return advance;
}

internal U32             
Utf16FromCodepoint(U16 *out, U32 codepoint)
{
    U32 advance = 1;
    if (codepoint == ~((U32)0))
    {
        out[0] = (U16)'?';
    }
    else if (codepoint < 0x10000)
    {
        out[0] = (U16)codepoint;
    }
    else
    {
        U64 v = codepoint - 0x10000;
        out[0] = (u16) (0xD800 + (v >> 10));
        out[1] = (u16) (0xDC00 + (v & bitmask10));
        advance = 2;
    }
    return advance;
}

internal String8         
Str8From16(M_Arena *arena, String16 in)
{
    U64 cap = in.size*3;
    U8 *str = PushArray(arena, U8, cap + 1);
    U16 *ptr = in.str;
    U16 *opl = ptr + in.size;
    U64 size = 0;
    DecodedCodepoint consume;
    for (;ptr < opl;)
    {
        consume = DecodeCodepointFromUtf16(ptr, opl - ptr);
        ptr += consume.advance;
        size += Utf8FromCodepoint(str + size, consume.codepoint);
    }
    str[size] = 0;
    M_ArenaPop(arena, cap - size); // := ((cap + 1) - (size + 1))
    return Str8(str, size);
}

internal String16        
Str16From8(M_Arena *arena, String8 in)
{
    U64 cap = in.size*2;
    U16 *str = PushArray(arena, U16, cap + 1);
    U8 *ptr = in.str;
    U8 *opl = ptr + in.size;
    U64 size = 0;
    DecodedCodepoint consume;
    for (;ptr < opl;)
    {
        consume = DecodeCodepointFromUtf8(ptr, opl - ptr);
        ptr += consume.advance;
        size += Utf16FromCodepoint(str + size, consume.codepoint);
    }
    str[size] = 0;
    M_ArenaPop(arena, 2*(cap - size)); // := 2*((cap + 1) - (size + 1))
    String16 result = {str, size};
    return result;
}

internal String8         
Str8From32(M_Arena *arena, String32 in)
{
    U64 cap = in.size*4;
    U8 *str = PushArray(arena, U8, cap + 1);
    U32 *ptr = in.str;
    U32 *opl = ptr + in.size;
    U64 size = 0;
    for (;ptr < opl; ptr += 1)
    {
        size += Utf8FromCodepoint(str + size, *ptr);
    }
    str[size] = 0;
    M_ArenaPop(arena, cap - size); // := ((cap + 1) - (size + 1))
    return Str8(str, size);
}

internal String32        
Str32From8(M_Arena *arena, String8 in)
{
    U64 cap = in.size;
    U32 *str = PushArray(arena, U32, cap + 1);
    U8 *ptr = in.str;
    U8 *opl = ptr + in.size;
    U64 size = 0;
    DecodedCodepoint consume;
    for (;ptr < opl;)
    {
        consume = DecodeCodepointFromUtf8(ptr, opl - ptr);
        ptr += consume.advance;
        str[size] = consume.codepoint;
        size += 1;
    }
    str[size] = 0;
    M_ArenaPop(arena, 4*(cap - size)); // := 4*((cap + 1) - (size + 1))
    String32 result = {str, size};
    return result;
}

////////////////////////////////
//~ rjf: Skip/Chop Helpers

internal String8
Str8ChopLastPeriod(String8 string)
{
    U64 period_pos = FindSubstr8(string, Str8Lit("."), 0, MatchFlag_FindLast);
    if(period_pos < string.size)
    {
        string.size = period_pos;
    }
    return string;
}

internal String8
Str8SkipLastSlash(String8 string)
{
    U64 slash_pos = FindSubstr8(string, Str8Lit("/"), 0,
                                MatchFlag_SlashInsensitive|
                                MatchFlag_FindLast);
    if(slash_pos < string.size)
    {
        string.str += slash_pos+1;
        string.size -= slash_pos+1;
    }
    return string;
}

internal String8
Str8SkipLastPeriod(String8 string)
{
    U64 period_pos = FindSubstr8(string, Str8Lit("."), 0, MatchFlag_FindLast);
    if(period_pos < string.size)
    {
        string.str += period_pos+1;
        string.size -= period_pos+1;
    }
    return string;
}

internal String8
Str8ChopLastSlash(String8 string)
{
    U64 slash_pos = FindSubstr8(string, Str8Lit("/"), 0,
                                MatchFlag_SlashInsensitive|
                                MatchFlag_FindLast);
    if(slash_pos < string.size)
    {
        string.size = slash_pos;
    }
    return string;
}

////////////////////////////////
//~ rjf: Numeric Conversions

internal U64
U64FromStr8(String8 string, U32 radix)
{
    Assert(2 <= radix && radix <= 16);
    local_persist U8 char_to_value[] =
    {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    };
    U64 value = 0;
    for (U64 i = 0; i < string.size; i += 1){
        value *= radix;
        U8 c = string.str[i];
        value += char_to_value[(c - 0x30)&0x1F];
    }
    return value;
}

internal S64
CStyleIntFromStr8(String8 string)
{
    U64 p = 0;
    
    // consume sign
    S64 sign = +1;
    if (p < string.size){
        U8 c = string.str[p];
        if (c == '-'){
            sign = -1;
            p += 1;
        }
        else if (c == '+'){
            p += 1;
        }
    }
    
    // radix from prefix
    U64 radix = 10;
    if (p < string.size){
        U8 c0 = string.str[p];
        if (c0 == '0'){
            p += 1;
            radix = 8;
            if (p < string.size){
                U8 c1 = string.str[p];
                if (c1 == 'x'){
                    p += 1;
                    radix = 16;
                }
                else if (c1 == 'b'){
                    p += 1;
                    radix = 2;
                }
            }
        }
    }
    
    // consume integer "digits"
    String8 digits_substr = Str8Skip(string, p);
    U64 n = U64FromStr8(digits_substr, (u32)radix);
    
    // combine result
    S64 result = sign*n;
    return(result);
}

internal F64
F64FromStr8(String8 string)
{
    char str[64];
    U64 str_size = string.size;
    if(str_size > sizeof(str) - 1)
    {
        str_size = sizeof(str) - 1;
    }
    MemoryCopy(str, string.str, str_size);
    str[str_size] = 0;
    return(atof(str));
}

internal String8
CStyleHexStringFromU64(M_Arena *arena, U64 x, B32 caps)
{
    local_persist char int_value_to_char[] = "0123456789abcdef";
    U8 buffer[10];
    U8 *opl = buffer + 10;
    U8 *ptr = opl;
    if (x == 0){
        ptr -= 1;
        *ptr = '0';
    }
    else
    {
        for(;;)
        {
            U32 val = x%16;
            x /= 16;
            U8 c = (U8)int_value_to_char[val];
            if(caps)
            {
                c = CharToUpper(c);
            }
            ptr -= 1;
            *ptr = c;
            if (x == 0)
            {
                break;
            }
        }
    }
    ptr -= 1;
    *ptr = 'x';
    ptr -= 1;
    *ptr = '0';
    
    String8 result = {0};
    result.size = (U64)(ptr - buffer);
    result.str = PushArray(arena, U8, result.size);
    MemoryCopy(result.str, buffer, result.size);
    
    return result;
}
