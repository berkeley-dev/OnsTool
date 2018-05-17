#include <algorithm>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

class OnsTool {
public:
	explicit OnsTool(const std::string &path) {
		mPath = path;
	}

	void Load() {
		try {
			std::ifstream file = std::ifstream(mPath, std::ios::binary | std::ios::ate);
			std::streamsize size = file.tellg();

			file.seekg(0, std::ios::beg);
			auto buffer = std::vector<char>(static_cast<size_t>(size));

			if (!file.read(buffer.data(), size)) {
				throw std::invalid_argument("Unable to read " + mPath);
			}

			for (size_t i = 0; i < buffer.size(); i += sizeof(spnData)) {
				mNetworks.emplace_back(*reinterpret_cast<spnData *>(buffer.data() + i));
			}
		} catch (std::bad_alloc &exception) {
			throw std::invalid_argument("Unable to open " + mPath);
		}
	}

	void Save() {
		std::ofstream file = std::ofstream(mPath, std::ofstream::binary);
		file.write(reinterpret_cast<const char *>(mNetworks.data()), mNetworks.size() * sizeof(spnData));
	}

	void List() {
		for (const auto &network : mNetworks) {
			std::cout << "mnc: " << network.mnc << ", "
					  << "mcc: " << network.mcc << ", "
					  << "name_1: " << network.name_1 << ", "
					  << "name_2: " << network.name_2
					  << std::endl;
		}
	}

	void Add(uint32_t mnc, uint32_t mcc, const std::string &name_1, const std::string &name_2) {
		auto it = std::find_if(mNetworks.begin(), mNetworks.end(), [&](const spnData &data) {
			return data.mnc == mnc && data.mcc == mcc;
		});

		if (it != mNetworks.end()) {
			return;
		}

		spnData data{};

		data.mnc = mnc;
		data.mcc = mcc;

		std::memcpy(&data.name_1, name_1.data(), sizeof(data.name_1));
		std::memcpy(&data.name_2, name_2.data(), sizeof(data.name_2));

		mNetworks.emplace_back(data);
	}

	void Edit(uint32_t mnc, uint32_t mcc, const std::string &name_1) {
		auto it = std::find_if(mNetworks.begin(), mNetworks.end(), [&](const spnData &data) {
			return data.mnc == mnc && data.mcc == mcc;
		});

		if (it == mNetworks.end()) {
			return;
		}

		std::memcpy(&it->name_1, name_1.data(), sizeof(it->name_1));
	}

	void Edit(uint32_t mnc, uint32_t mcc, const std::string &name_1, const std::string &name_2) {
		auto it = std::find_if(mNetworks.begin(), mNetworks.end(), [&](const spnData &data) {
			return data.mnc == mnc && data.mcc == mcc;
		});

		if (it == mNetworks.end()) {
			return;
		}

		std::memcpy(&it->name_1, name_1.data(), sizeof(it->name_1));
		std::memcpy(&it->name_2, name_2.data(), sizeof(it->name_2));
	}

	void Remove(uint32_t mnc, uint32_t mcc) {
		auto it = std::find_if(mNetworks.begin(), mNetworks.end(), [&](const spnData &data) {
			return data.mnc == mnc && data.mcc == mcc;
		});

		if (it != mNetworks.end()) {
			mNetworks.erase(it);
		}
	}
private:
	struct spnData {
		uint32_t mnc;
		uint32_t mcc;
		char name_1[128];
		char name_2[128];
	};

	std::string mPath;
	std::vector<spnData> mNetworks;
};

int main(int argc, char *argv[]) {
	auto showUsage = [](const char *programName) {
		std::cout << "usage: " << basename(programName) << " [path] [command] [optional parameters]" << std::endl;
		std::cout << "commands:" << std::endl;
		std::cout << "\tlist" << std::endl;
		std::cout << "\tadd [mnc] [mcc] [name_1] (name_2)" << std::endl;
		std::cout << "\tedit [mnc] [mcc] [name_1] (name_2)" << std::endl;
		std::cout << "\tremove [mnc] [mcc]" << std::endl;
	};
	auto add = [&](int argc, char *argv[]) {
		if (argc < 6) {
			showUsage(argv[0]);
			return;
		}

		OnsTool onsTool(argv[1]);
		onsTool.Load();

		auto mnc = static_cast<uint32_t>(std::stoi(argv[3]));
		auto mcc = static_cast<uint32_t>(std::stoi(argv[4]));
		auto name_1 = std::string(argv[5]);
		auto name_2 = argc > 6 ? std::string(argv[6]) : name_1;

		onsTool.Add(mnc, mcc, name_1, name_2);
		onsTool.Save();
	};
	auto edit = [&](int argc, char *argv[]) {
		if (argc < 6) {
			showUsage(argv[0]);
			return;
		}

		OnsTool onsTool(argv[1]);
		onsTool.Load();

		auto mnc = static_cast<uint32_t>(std::stoi(argv[3]));
		auto mcc = static_cast<uint32_t>(std::stoi(argv[4]));
		auto name_1 = std::string(argv[5]);
		auto name_2 = argc > 6 ? std::string(argv[6]) : name_1;

		if (argc > 6) {
			onsTool.Edit(mnc, mcc, name_1, name_2);
		} else {
			onsTool.Edit(mnc, mcc, name_1);
		}
		onsTool.Save();
	};
	auto list = [&](int argc, char *argv[]) {
		OnsTool onsTool(argv[1]);
		onsTool.Load();
		onsTool.List();
	};
	auto remove = [&](int argc, char *argv[]) {
		if (argc < 5) {
			showUsage(argv[0]);
			return;
		}

		OnsTool onsTool(argv[1]);
		onsTool.Load();

		auto mnc = static_cast<uint32_t>(std::stoi(argv[3]));
		auto mcc = static_cast<uint32_t>(std::stoi(argv[4]));

		onsTool.Remove(mnc, mcc);
		onsTool.Save();
	};

	if (argc < 3) {
		showUsage(argv[0]);
		return -1;
	}

	std::string command = argv[2];
	std::map<std::string, std::function<void(int argc, char *argv[])>> commands = {
			{"add", add},
			{"edit", edit},
			{"list", list},
			{"remove", remove},
	};

	auto it = commands.find(command);

	if (it == commands.end()) {
		showUsage(argv[0]);
		return -1;
	}

	it->second(argc, argv);

	return 0;
}
