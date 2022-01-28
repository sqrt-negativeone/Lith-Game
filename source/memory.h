
typedef struct M_Arena M_Arena;
struct M_Arena
{
    void *base;
    u64 max;
    u64 alloc_position;
    u64 commit_position;
};
