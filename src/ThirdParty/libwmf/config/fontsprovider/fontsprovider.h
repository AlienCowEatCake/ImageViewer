#if !defined (FONTSPROVIDER_H_INCLUDED)
#define FONTSPROVIDER_H_INCLUDED

#if defined (__cplusplus)
extern "C" {
#endif

const char *ProvideWmfFontdir(void);
const char *ProvideWmfGsFontdir(void);
const char *ProvideWmfSysFontmap(void);
const char *ProvideWmfXtraFontmap(void);
const char *ProvideWmfGsFontmap(void);

#if defined (__cplusplus)
}
#endif

#endif
