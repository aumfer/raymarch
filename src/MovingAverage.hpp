template<size_t Count>
class MovingAverage {
	static_assert((Count & (Count - 1)) == 0, "MovingAverage Count must be a power of 2");
public:
	MovingAverage() :
		index(0),
		avg(0) {
	};
	MovingAverage(const MovingAverage&) = delete;
	MovingAverage& operator=(const MovingAverage&) = delete;

	void Update(float dt) {
		float sum = 0;

		buffer[index++ & (Count - 1)] = dt;
		auto count = index > Count ? Count : index;
		for (auto i = 0u; i < count; ++i) {
			sum += buffer[i];
		}
		avg = sum / count;
	}

	float Average() const {
		return avg;
	}

private:
	float buffer[Count];
	unsigned index;
	float avg;
};