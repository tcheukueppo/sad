#undef reallocarray
void *reallocarray(void *, size_t, size_t);

#undef strlcat
size_t strlcat(char *, const char *, size_t);
#undef strlcpy
size_t strlcpy(char *, const char *, size_t);

#undef strtonum
long long strtonum(const char *, long long, long long, const char **);
