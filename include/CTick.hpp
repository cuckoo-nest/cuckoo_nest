#pragma once

#include <mutex>
#include <time.h>

class CTickFuture
{
public:
	CTickFuture() : mTickMs(0), mDefault(0) {};
	CTickFuture(unsigned short msInFuture) : mTickMs(0), mDefault(msInFuture) { ScheduleMs(msInFuture); };
	~CTickFuture() = default;

	CTickFuture const &operator =(CTickFuture const &rhs)
	{
		mTickMs = rhs.mTickMs;
		mDefault = rhs.mDefault;
		return *this;
	};

	static unsigned long Tick(void)
	{
		unsigned long rc = 0;
		struct timespec now;

		if(clock_gettime(CLOCK_MONOTONIC, &now) == 0)
		{
			rc = now.tv_sec * 1000; // to ms
			rc += now.tv_nsec / (1000 * 1000); // to ms
		}

		return rc;
	}

	static inline unsigned long Ms() { return CTickFuture::Tick(); };
	static inline unsigned long Sec() { return Ms() / 1000; };

	static inline unsigned long Ms(unsigned short msInFuture) { return Ms() + msInFuture; };
	static inline unsigned long Sec(unsigned short secsInFuture) { return Ms(secsInFuture * 1000); };

	inline bool IsExpired() { std::lock_guard<std::mutex> lock(mMutex); return (mTickMs != 0 && Ms() >= mTickMs); };
	inline bool IsScheduled() { std::lock_guard<std::mutex> lock(mMutex); return (mTickMs != 0); };

	inline void Clear() { ScheduleMs(0); };

	inline void Reset()
	{
		std::lock_guard<std::mutex> lock(mMutex); 
		unsigned long m = Ms();
		unsigned int t = mDefault;

		// This is about trying to keep the new expiration
		// time from drifting into the future, assuming that
		// the interval is a regularly occuring event (which
		// is what Reset() is supposed to be for) by biasing
		// the new expiration time, by the difference of
		// now minus the last expiration time, if that
		// difference is less than the expiration interval,
		// and only if it has expired, already.
		// Otherwise, sa la vie.
		if(mTickMs != 0 && m >= mTickMs)
		{
			unsigned int diff = m - mTickMs;

			// bias ?
			if(diff < t)
				t -= diff;
		}

		mTickMs = m + t;
	};
	inline void Default(unsigned int v) { mDefault = v; };
	inline unsigned int Default() { return mDefault; };

	inline void ScheduleMs(unsigned short v) { std::lock_guard<std::mutex> lock(mMutex); mTickMs = (v != 0 ? Ms(v) : 0); };
	inline void ScheduleSec(unsigned short v) { ScheduleMs(v * 1000); };
protected:
	unsigned long mTickMs = 0;
	unsigned int mDefault = 0;
	std::mutex mMutex;
};

class CTickDiff
{
public:
	CTickDiff() { mLast = {0}; };
	~CTickDiff() = default;

	long diff()
	{
		long rc = 0;
		struct timespec now;

		if(clock_gettime(CLOCK_MONOTONIC, &now) == 0)
		{
			if(mLast.tv_sec > 0)
			{
				rc = (now.tv_sec - mLast.tv_sec) * 1000; // to ms
				rc += (now.tv_nsec - mLast.tv_nsec) / (1000 * 1000); // to ms
			}

			mLast = now;
		}
		return rc;
	}
private:
	struct timespec mLast;
};
