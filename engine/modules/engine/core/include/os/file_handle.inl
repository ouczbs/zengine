namespace api{
    template<typename T>
	inline T FileHandle::ReadLeft()
	{
		T data{pool};
		auto& fi = Reader();
		size_t read_size = size - fi.tellg();
		data.resize(read_size, '\0');
		fi.read(data.data(), read_size);
		// 检查实际读取的字节数
		size_t count = (size_t)fi.gcount();
		if (count != read_size) {
			data.resize(count);
		}
		return data;
	}
	template<typename T>
	inline T FileHandle::ReadUntil(const T& token)
	{
		auto& fi = Reader();
		size_t start = fi.tellg();
		size_t stop = size;
		size_t skip_to = 0;
		if (FindNext(token.data(), &token.back(), stop)) {
			skip_to = stop + token.size();
		}
		T buffer{pool};
		buffer.resize(stop - start);
		fi.read(buffer.data(), stop - start);
		size_t count = (size_t)fi.gcount();
		if (count != stop - start) {
			buffer.resize(count);
		}
		if (skip_to) {
			fi.seekg(skip_to);
		}
		return buffer;
	}
}