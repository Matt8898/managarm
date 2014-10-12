
void *operator new (size_t size, void *pointer);
extern "C" void __cxa_pure_virtual();

extern "C" void thorRtHalt();

extern "C" void thorRtInvalidatePage(void *pointer);
extern "C" void thorRtInvalidateSpace();

template<typename T>
class LazyInitializer {
public:
	template<typename... Args>
	void initialize(Args... args) {
		new(p_object) T(args...);
	}

	T *access() {
		return (T *)p_object;
	}
	T *operator->() {
		return access();
	}

private:
	char p_object[sizeof(T)];
};

