#ifndef os_thread_h
#define os_thread_h

#if defined(ARDUINO_ARCH_RP2040)

typedef semaphore_t SemaphoreObject;
static inline void SemaphoreInit(SemaphoreObject &obj, int16_t initial_permits, int16_t max_permits) {
	sem_init(&obj, initial_permits, max_permits);
}
static inline int SemaphoreTake(SemaphoreObject &obj) {
	return sem_acquire_blocking(&obj);
}
static inline int SemaphoreGive(SemaphoreObject &obj) {
	return sem_release(&obj);
}

typedef mutex_t MutexObject;
static inline void MutexInit(MutexObject &obj) {
	mutex_init(&obj);
}
static inline void MutexLock(MutexObject &obj) {
	mutex_enter_blocking(obj);
}
static inline void MutexUnlock(MutexObject &obj) {
	mutex_exit(&obj);
}

#elif defined(ARDUINO_ARCH_ESP32)

typedef SemaphoreHandle_t SemaphoreObject;
static inline void SemaphoreInit(SemaphoreObject &obj, int16_t initial_permits, int16_t max_permits) {
	obj = xSemaphoreCreateCounting(max_permits, initial_permits);
}
static inline int SemaphoreTake(SemaphoreObject &obj) {
	return xSemaphoreTake(obj, portMAX_DELAY);
}
static inline int SemaphoreGive(SemaphoreObject &obj) {
	return xSemaphoreGive(obj);
}

typedef SemaphoreHandle_t MutexObject;
static inline void MutexInit(MutexObject &obj) {
	obj = xSemaphoreCreateMutex();
}
static inline void MutexLock(MutexObject &obj) {
	xSemaphoreTake(obj, portMAX_DELAY);
}
static inline void MutexUnlock(MutexObject &obj) {
	xSemaphoreGive(obj);
}

#endif

#endif  // os_thread_h