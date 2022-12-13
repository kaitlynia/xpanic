#ifndef TEEOTHER_COMPONENTS_LOCALIZATION_H
#define TEEOTHER_COMPONENTS_LOCALIZATION_H

#include <teeother/tl/hashtable.h>

#include <cstdarg>
#include <string>
#include <type_traits>

class CLocalization
{
	class IStorage* m_pStorage;
	IStorage* Storage() { return m_pStorage; }

public:

	class CLanguage
	{
	protected:
		class CEntry
		{
		public:
			char* m_apVersions;

			CEntry() : m_apVersions(nullptr) {}

			void Free()
			{
				if (m_apVersions)
				{
					delete[] m_apVersions;
					m_apVersions = nullptr;
				}
			}
		};

		char m_aName[64];
		char m_aFilename[64];
		char m_aParentFilename[64];
		bool m_Loaded;
		int m_Direction;

		hashtable< CEntry, 128 > m_Translations;

	public:
		CLanguage();
		CLanguage(const char* pName, const char* pFilename, const char* pParentFilename);
		~CLanguage();

		const char* GetParentFilename() const { return m_aParentFilename; }
		const char* GetFilename() const { return m_aFilename; }
		const char* GetName() const { return m_aName; }
		bool IsLoaded() const { return m_Loaded; }
		bool Load(CLocalization* pLocalization, IStorage* pStorage);
		const char* Localize(const char* pKey) const;
	};

	enum
	{
		DIRECTION_LTR=0,
		DIRECTION_RTL,
		NUM_DIRECTIONS,
	};

protected:
	CLanguage* m_pMainLanguage;

public:
	array<CLanguage*> m_pLanguages;
	fixed_string128 m_Cfg_MainLanguage;

protected:
	const char* LocalizeWithDepth(const char* pLanguageCode, const char* pText, int Depth);

public:
	CLocalization(IStorage* pStorage);
	virtual ~CLocalization();

	virtual bool InitConfig(int argc, const char** argv);
	virtual bool Init();

	//localize
	const char* Localize(const char* pLanguageCode, const char* pText);

	// translate to commas
	template<typename integer, const char separator = ',', const unsigned num = 3>
	static std::enable_if_t<std::is_integral_v<integer>, std::string > GetCommas(integer Number)
	{
		std::string NumberString = std::to_string(Number);

		if(NumberString.length() > num)
		{
			for(auto it = NumberString.rbegin(); (num + 1) <= std::distance(it, NumberString.rend());)
			{
				std::advance(it, num);
				NumberString.insert(it.base(), separator);
			}
		}

		return NumberString;
	}

	//format
	void Format_V(dynamic_string& Buffer, const char* pLanguageCode, const char* pText, va_list VarArgs);
	void Format(dynamic_string& Buffer, const char* pLanguageCode, const char* pText, ...);
	//localize, format
	void Format_VL(dynamic_string& Buffer, const char* pLanguageCode, const char* pText, va_list VarArgs);
	void Format_L(dynamic_string& Buffer, const char* pLanguageCode, const char* pText, ...);
};

#endif