class Timer {
public:
	Timer();

	void Update();

	float Delta() const;
	float Time() const;
	float Elapsed() const;

private:
	float prev, time, elapsed, dt;
};