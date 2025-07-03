#ifndef CONFIG_HPP
#define CONFIG_HPP

class Config {
	private:
		int _listen;

	public:
		Config(int listen);

		int getListen();
};

#endif
