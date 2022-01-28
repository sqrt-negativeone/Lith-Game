
//~ NOTE(rjf): String Manipulation

#define CharIsSpace(c) ((c) <= 32)
#define CharIsDigit CharIsNumeric

internal b32
CharIsAlpha(char c)
{
    return ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z'));
}

internal b32
CharIsDigit(char c)
{
    return (c >= '0' && c <= '9');
}

internal b32
CharIsSymbol(char c)
{
    return (c == '~' ||
            c == '!' ||
            c == '%' ||
            c == '^' ||
            c == '&' ||
            c == '*' ||
            c == '(' ||
            c == ')' ||
            c == '[' ||
            c == ']' ||
            c == '{' ||
            c == '}' ||
            c == '-' ||
            c == '+' ||
            c == '=' ||
            c == ';' ||
            c == ':' ||
            c == '<' ||
            c == '>' ||
            c == '/' ||
            c == '?' ||
            c == '.' ||
            c == ',');
}

internal char
CharToLower(char c)
{
    if(c >= 'A' && c <= 'Z')
    {
        return c + 32;
    }
    return c;
}

internal char
CharToUpper(char c)
{
    if(c >= 'a' && c <= 'z')
    {
        return c - 32;
    }
    return c;
}

typedef struct String8 String8;
typedef struct String8 S8;
typedef S8 s8;
struct String8
{
    union
    {
        char *string;
        char *str;
        void *data;
        void *ptr;
    };
    u64 size;
};

#define S8Lit(s) S8{(char*)(s), ArrayCount(s) - 1}
#define S8LitComp(s) {(char*)(s), ArrayCount(s) - 1}
#define StringExpand(s) (int)((s).size), ((s).str)

typedef u32 StringMatchFlags;
enum
{
    StringMatchFlag_MatchCase       = (1<<0),
    StringMatchFlag_RightSideSloppy = (1<<1),
};
