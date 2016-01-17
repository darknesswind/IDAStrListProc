#pragma once

class RawStrReader
{
public:
	RawStrReader(ea_t begin, ea_t end)
		: m_begin(begin), m_end(end)
	{
		m_curAddress = m_begin;
		m_buffPos = m_begin;
		m_curBuffSize = 0;
		readNextBuff();
	}

	ea_t curAddr() const { return m_curAddress; }
	bool readNext()
	{
		if (!seekToNotNil())
			return false;

		m_curAddress = m_buffPos + m_posInBuff;
		m_readed.clear();
		bool bFound = false;
		do
		{
			while (m_posInBuff < m_curBuffSize)
			{
				BYTE ch = m_buff[m_posInBuff];
				if (0 == ch)
				{
					bFound = true;
					break;
				}
				m_readed.push_back(ch);
				++m_posInBuff;
			}
			if (!bFound)
			{
				if (!readNextBuff())
					break;
			}

		} while (!bFound);
		return bFound;
	}

	std::vector<BYTE>& getBytes() { return m_readed; }

protected:
	bool readNextBuff()
	{
		m_posInBuff = 0;
		m_buffPos += m_curBuffSize;
		m_curBuffSize = min(m_end - m_buffPos, s_buffSize);
		if (!m_curBuffSize)
			return false;

		return get_many_bytes(m_buffPos, m_buff.data(), m_curBuffSize);
	}

	bool seekToNotNil()
	{
		bool bFound = false;
		do
		{
			while (m_posInBuff < m_curBuffSize)
			{
				if (0 != m_buff[m_posInBuff])
				{
					bFound = true;
					break;
				}
				++m_posInBuff;
			}
			if (!bFound)
			{
				if (!readNextBuff())
					break;
			}

		} while (!bFound);

		return bFound;
	}

private:
	static const int s_buffSize = 0x200;
	std::array<BYTE, s_buffSize> m_buff;
	std::vector<BYTE> m_readed;
	ea_t m_begin = 0;
	ea_t m_end = 0;
	ea_t m_buffPos = 0;
	ea_t m_curAddress = 0;
	uint m_posInBuff = 0;
	uint m_curBuffSize = 0;
};

class Utf16Reader
{
public:
	Utf16Reader(FILE* hFile)
		: m_hFile(hFile)
		, m_curAddress(0)
		, m_buffPos(0)
		, m_curBuffSize(0)
		, m_fileSize(0)
	{
		const WORD bom = 0xFEFF;
		WORD buff = 0;
		eread(hFile, &buff, sizeof(buff));
		if (buff != bom)
		{
			eclose(hFile);
			return;
		}
		m_buffPos += sizeof(buff);
		m_fileSize = qfsize(hFile);
		readNextBuff();
	}

	uint curAddr() const { return m_curAddress; }
	bool readNext()
	{
		m_curAddress = m_buffPos + m_posInBuff * sizeof(wchar16_t);
		m_readed.clear();
		bool bPart1 = false;
		bool bPart2 = false;
		do
		{
			while (!bPart2 && m_posInBuff < m_curBuffSize)
			{
				wchar16_t ch = m_buff[m_posInBuff];
				if (L'\r' == ch)
				{
					bPart1 = true;
				}
				else if (bPart1)
				{
					if (L'\n' == ch)
						bPart2 = true;
				}
				else
				{
					m_readed.append(ch);
				}
				++m_posInBuff;
			}
			if (!bPart2)
			{
				if (!readNextBuff())
					break;
			}

		} while (!bPart2);
		return bPart2;
	}

	qwstring& getLine() { return m_readed; }

protected:
	bool readNextBuff()
	{
		m_posInBuff = 0;
		m_buffPos += m_curBuffSize * sizeof(wchar16_t);

		uint sizeToRead = min(m_fileSize - m_buffPos, s_buffSize * sizeof(wchar16_t));
		if (!sizeToRead)
			return false;

		eread(m_hFile, m_buff.data(), sizeToRead);
		m_curBuffSize = sizeToRead / sizeof(wchar16_t);
		return true;
	}

private:
	static const int s_buffSize = 0x200;
	std::array<wchar16_t, s_buffSize> m_buff;
	qwstring m_readed;
	FILE* m_hFile;
	uint m_fileSize;
	uint m_buffPos = 0;
	uint m_curAddress = 0;
	uint m_posInBuff = 0;
	uint m_curBuffSize = 0;
};
