#pragma once

namespace Dashlane
{

	struct DashlaneContextInternal;

	bool SetLocalKey(DashlaneContextInternal& context);
	bool GetLocalKey(DashlaneContextInternal& context);
	bool DeleteLocalKey(DashlaneContextInternal& context);
	EDashlaneError GetOrUpdateSecrets(DashlaneContextInternal& context);
	EDashlaneError UpdateDeviceConfiguration(DashlaneContextInternal& context);

}