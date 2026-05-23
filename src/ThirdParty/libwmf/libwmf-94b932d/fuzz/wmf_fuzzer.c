/* libwmf libFuzzer harness, intended for both local fuzzing and oss-fuzz.
 *
 * Exercises the metafile parser via wmf_mem_open + wmf_scan. We use the eps
 * backend purely to satisfy wmf_api_create's requirement for a function
 * reference - wmf_scan only walks records to compute bounds, it does not
 * actually emit any output, so the choice of backend is mostly cosmetic.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <libwmf/api.h>
#include <libwmf/eps.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	if (size == 0)
		return 0;

	unsigned long flags = WMF_OPT_NO_DEBUG | WMF_OPT_NO_ERROR
	                      | WMF_OPT_IGNORE_NONFATAL | WMF_OPT_FUNCTION;

	wmfAPI_Options api_options;
	memset(&api_options, 0, sizeof(api_options));
	api_options.function = wmf_eps_function;

	wmfAPI *API = NULL;
	if (wmf_api_create(&API, flags, &api_options) != wmf_E_None || API == NULL)
	{
		if (API) wmf_api_destroy(API);
		return 0;
	}

	unsigned char *buf = (unsigned char *) malloc(size);
	if (!buf)
	{
		wmf_api_destroy(API);
		return 0;
	}
	memcpy(buf, data, size);

	if (wmf_mem_open(API, buf, (long) size) == wmf_E_None)
	{
		wmfD_Rect bbox;
		wmf_scan(API, 0, &bbox);
		wmf_mem_close(API);
	}

	wmf_api_destroy(API);
	free(buf);
	return 0;
}
