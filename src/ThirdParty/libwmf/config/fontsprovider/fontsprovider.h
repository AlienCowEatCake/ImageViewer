#if !defined (FONTSPROVIDER_H_INCLUDED)
#define FONTSPROVIDER_H_INCLUDED

#if defined (__cplusplus)
extern "C" {
#endif

const char *ProvideWmfFontdir();
const char *ProvideWmfGsFontdir();
const char *ProvideWmfSysFontmap();
const char *ProvideWmfXtraFontmap();
const char *ProvideWmfGsFontmap();

#if defined (__cplusplus)
}
#endif

#endif
