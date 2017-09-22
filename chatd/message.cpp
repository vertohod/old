#include <boost/regex.hpp>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <stdexcept>
#include <unistd.h>
#include <string>
#include <ctime>

#include "service_messages.h"
#include "stat_daemon.h"
#include "configure.h"
#include "alphabet.h"
#include "message.h"
#include "crypto.h"
#include "stream.h"
#include "domain.h"
#include "error.h"
#include "image.h"
#include "smile.h"
#include "util.h"
#include "log.h"

namespace chatd
{

DEFINITION_FIELDS_FUNCTIONS_STORES(message)

DEFINITION_CONSTS(message, "message", true, 1)

DEFINITION_FIELD(message, OID, poid)
DEFINITION_FIELD(message, OID, oid_from)
DEFINITION_FIELD(message, OID, oid_to)
DEFINITION_FIELD(message, bool, private)
DEFINITION_FIELD(message, time_t, time)
DEFINITION_FIELD(message, unsigned char, color)
DEFINITION_FIELD(message, sptr_cstr, avatar)
DEFINITION_FIELD(message, sptr_cstr, name_from)
DEFINITION_FIELD(message, sptr_cstr, name_to)
DEFINITION_FIELD(message, sptr_cstr, message)
DEFINITION_FIELD(message, sptr_cstr, raw)

DEFINITION_FIELD(message, std::shared_ptr<std::vector<OID>>, images)
//DEFINITION_FIELD(message, std::shared_ptr<std::vector<OID>>, audios)
//DEFINITION_FIELD(message, std::shared_ptr<std::vector<OID>>, videos)
//DEFINITION_FIELD(message, std::shared_ptr<std::vector<OID>>, archives)

DEFINITION_FIELD(message, std::shared_ptr<std::vector<sptr_str>>, youtube)

message::message()
{
	FIELD(poid) = 0;
	FIELD(oid_from) = 0;
	FIELD(oid_to) = 0;
	FIELD(private) = 0;
	FIELD(time) = 0;
	FIELD(color) = 0;

	FIELD(images) = std::make_shared<std::vector<OID>>();
	FIELD(youtube) = std::make_shared<std::vector<sptr_str>>();
}
/*
	proc_message_interval::start();

	m_time = util::get_gtime();

	std::string message_temp;

	m_images = filter_medias<chatd::image_store>(oid_from, oid_to, images);
	m_audios = filter_medias<chatd::audio_store>(oid_from, oid_to, audios);
	m_videos = filter_medias<chatd::video_store>(oid_from, oid_to, videos);
	m_archives = filter_medias<chatd::archive_store>(oid_from, oid_to, archives);

	if (oid_from == 0) processing_service_message(message);
	else processing_message(message);

	proc_message_interval::finish();
*/

std::unique_ptr<blizzard::keys_stores_t> message::init_keys()
{
	std::unique_ptr<blizzard::keys_stores_t> res(new blizzard::keys_stores_t());

	// Добавляем ключи

	return res;
}

uptr_str message::serialization() const
{
	auto res = object::serialization();
    sp::stream st(*res);

    for (auto& pr : m_serialization_functions) {
        st << pr.first << "=" << pr.second(this) << SEPARATOR;
    }

    return res;
}

bool message::init_field(const std::string& key, const std::string& val)
{
    auto it = m_deserialization_functions.find(key);

    if (it != m_deserialization_functions.end()) {
        it->second(this, val);
        return true;
    }

	return object::init_field(key, val);
}

std::shared_ptr<blizzard::object> message::make_copy() const
{
	std::shared_ptr<blizzard::object> obj_new = create();

	copy_fields(*obj_new);

	return obj_new;
}

void message::copy_fields(blizzard::object& obj) const
{
	object::copy_fields(obj);

	for (auto& pr : m_serialization_functions) {
		dynamic_cast<message*>(&obj)->init_field(pr.first, pr.second(this));
	}
}

template <typename T>
std::shared_ptr<std::vector<OID>> message::filter_medias(OID from, OID to, sptr_cstr oids_list_str)
{
	std::shared_ptr<std::vector<OID>> res;
	if (oids_list_str->empty()) return res;

	auto& store = singleton<T>::instance();

	auto oids_vector = util::split(*oids_list_str, ",");

	for (auto& oid_str : *oids_vector) {
		auto oid = format<OID>(oid_str);

		auto obj_ref = store.get(oid);
		if (obj_ref->check_owner(from)) {
			obj_ref->add_allow(to);
			res->push_back(oid);
		}
	}
	return res;
}

void message::processing_service_message(sptr_cstr arg_message)
{
	auto pos = arg_message->find(" ");
	if (pos != std::string::npos) {

		auto mess = arg_message->substr(pos + 1, arg_message->size() - pos - 1);
		auto vec = util::split(mess, ";");

		if (vec->size() > 0) FIELD(message) = vec->at(0);
	}
}

void message::processing_message(sptr_cstr arg_message)
{
	static std::string rus_alph(ALPH_RUSSION);
    static boost::regex reg_url("^((https?|ftp)\\:\\/\\/)?([\\.a-z0-9" + rus_alph + "-]+\\.([a-z" + rus_alph + "]{2,14}))(\\/?(.*))$");
    static boost::regex reg_email("^([a-z0-9_-]+\\.)*[a-z0-9_-]+@[a-z0-9_-]+(\\.[a-z0-9_-]+)*\\.[a-z]{2,6}$");

	static std::string domain_youtube1("youtube.com");
	static std::string domain_youtube2("www.youtube.com");
	static std::string domain_youtube3("youtu.be");

	std::vector<std::string> message_ls;
	split_message(*arg_message, message_ls);

	auto& smile = singleton<smile_store>::instance();
	auto& domain = singleton<domain_store>::instance();

	bool space_flag = false;

	sptr_str res;
	for (auto word_it = message_ls.begin(); word_it != message_ls.end();) {

		auto& word = *word_it;

		if (word.size() > 1) {

			boost::match_results<std::string::const_iterator> what;

			bool flag_smile = false;

			// осуществляем поиск смайлов
			auto smile_file = smile.get(word);
			// после поиска квотируем html-символы
			if (!smile_file.empty()) {
				word = "<img src=\"" + cfg().dir_smiles_short + smile_file
					+ "\" alt=\"" + *util::escape_html(word)
					+ "\" title=\"" + *util::escape_html(word) + "\"/>";

				flag_smile = true;
			} else {
				// Если смайл в слове не найден, дополнительно обрабатываем слово
				// с поиском скобок и спец-символов
				std::vector<std::string> message_ls_temp;
				if (1 < split_message(*word_it, message_ls_temp, true)) {
					auto it_temp = word_it;

					++it_temp;
					message_ls.insert(it_temp, message_ls_temp.begin(), message_ls_temp.end());

					++word_it;
					continue;
				}
			}

			// Если смайл уже был найден, проверять больше не надо
			if (!flag_smile) {

				if (boost::regex_search(word, what, reg_url)) {

					// Если распознаем youtube url, то добавляем его в отдельный буффер
					if (what[3] == domain_youtube1 ||
						what[3] == domain_youtube2 ||
						what[3] == domain_youtube3) {

						try {

							FIELD(youtube)->push_back(make_youtube_url(word));

							++word_it;
							continue;

						} catch (int e) {
							// Если что-то пошло не так, добавляем как обычную ссылку
						}
					}

					// Дополнительная проверка существования домена первого уровня
					if (domain.check(what[4])) {
						// если протокол не указан, добавляем
						if (word.substr(0, 4) != "http" && word.substr(0, 3) != "ftp") {
							word = "http://" + word;
						}
						// url найден
						word = "<a href=\"" + *util::escape_html(word)
							+ "\" title=\"" + *util::escape_html(word) + "\" target=\"_blank\">" + *util::escape_html(what[3]) + "</a>";
					} else word = *util::escape_html(word);

				} else if (boost::regex_match(word, reg_email)) {
					// email найден
					word = "<a href=\"mailto:" + *util::escape_html(word)
						+ "\" title=\"" + *util::escape_html(word) + "\">" + *util::escape_html(word) + "</a>";
				} else word = *util::escape_html(word);
			}

		} else word = *util::escape_html(word); // конец условия строки > 1

		// Пропускаем повторные пробелы
		if (word == " ") {
			if (space_flag) {
				++word_it;
				continue;
			}
			space_flag = true;
		} else space_flag = false;

		res->append(word);

		++word_it;
	}

	FIELD(message) = res;
}

size_t message::split_message(const std::string& text, std::vector<std::string>& out, bool bracket)
{
	enum {
		NOTHING,
		LATIN,
		SPACE,
		DOT,
		COMMA,
		OTHER
	} state = NOTHING;

    setlocale(LC_ALL, "ru_RU.UTF-8");
    int length = text.length();

	int word_begin = 0;
	int size = 0;

    for (int offset = 0; offset < length; offset += size) {

		size = mblen(&text.at(offset), length - offset);

		if (size != 1 && bracket) continue;

		switch (size) {

			case -1: {
				size = 1;
				continue;
			}
			case 0: {
				if (length - word_begin > 0) out.push_back(text.substr(word_begin, length - word_begin));
				return out.size();
			}
			case 1: {

				char ch = text.at(offset);

				if (bracket) {
					switch (ch) {
						case '(':
						case ')':
						case '[':
						case ']':
						case '{':
						case '}':
						case '<':
						case '>':
						case '"':
						case '\'':
						case '|':
						case ';':
						{
							if (offset - word_begin > 0) out.push_back(text.substr(word_begin, offset - word_begin));
							out.push_back(std::string(1, ch));
							word_begin = offset + 1;
							continue;
						}
						default:
							break;
					}
				} else {

// После запятых добавлялся пробел. Попробуем пока его не добавлять
//					if (state == COMMA && ch != ',' && !(ch <= 0x20 || ch == 0x7f)) out.push_back(" ");

					switch (ch) {
						case '.':
							if (state == DOT) {
								if (offset - word_begin - 1 > 0) {
									out.push_back(text.substr(word_begin, offset - word_begin - 1));
								}
								word_begin = offset - 1;
							} else state = DOT;
							// Если конец строки, то точку надо оторвать от предыдущего слова (не входит в url)
							if (offset + 1 == length) {
								if (offset - word_begin > 0) {
									out.push_back(text.substr(word_begin, offset - word_begin));
								}
								word_begin = offset;
							}
							continue;
						case ',': {
							if (state != COMMA) {
								if (offset - word_begin > 0) out.push_back(text.substr(word_begin, offset - word_begin));
								out.push_back(",");
								state = COMMA;
							}
							word_begin = offset + 1;
							continue;
						}
						default:
							break;
					}

					if (ch <= 0x20 || ch == 0x7f) {

						switch (state) {
							case SPACE:
								word_begin = offset + 1;
								continue;
							case DOT: {
								if (offset - word_begin - 1 > 0) {
									out.push_back(text.substr(word_begin, offset - word_begin - 1));
								}
								word_begin = offset - 1;
								// break намеренно отсутствует
							}
							default: {
								if (offset - word_begin > 0) out.push_back(text.substr(word_begin, offset - word_begin));
								out.push_back(" ");
								word_begin = offset + 1;
								state = SPACE;
								continue;
							}
						}

					} else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {

						if (state == OTHER) {
							if (offset - word_begin > 0) out.push_back(text.substr(word_begin, offset - word_begin));
							out.push_back(" ");

							word_begin = offset;
						}
						state = LATIN;
						continue;

					}  else {

						state = NOTHING;
						continue;

					}
				}

				break;
			}
			case 3: {
				if (offset - word_begin > 0) out.push_back(text.substr(word_begin, offset - word_begin));
				out.push_back(text.substr(offset, 3));

				word_begin = offset + 3;
				state = OTHER;
				break;
			}
			case 4: {
				if (offset - word_begin > 0) out.push_back(text.substr(word_begin, offset - word_begin));
				out.push_back(text.substr(offset, 4));

				word_begin = offset + 4;
				state = OTHER;
				break;
			}
			// Символы с длиной >1 байта
			default: {

				switch (state) {
					case COMMA:
						out.push_back(" ");
						break;

					case LATIN: {
						if (offset - word_begin > 0) out.push_back(text.substr(word_begin, offset - word_begin));
						out.push_back(" ");

						word_begin = offset;
						break;
					}
					default:
						break;
				}

				state = OTHER;
				continue;
			}
		} // end of switch
	} // end of for
	if (length - word_begin > 0) out.push_back(text.substr(word_begin, length - word_begin));

	return out.size();
}

sptr_str message::make_youtube_url(const std::string& url)
{
	static std::string base_url("https://www.youtube.com/embed/");
	static std::string option_name("v=");

	sptr_str res;

	auto pos_b = url.find(option_name);
	auto pos_s = url.rfind("/");

	if (pos_b != std::string::npos) {
		auto pos_e = url.find("&", pos_b);

		size_t length;
		if (pos_e != std::string::npos) {
			length = pos_e - pos_b - option_name.length();
		} else {
			length = url.length() - pos_b - option_name.length();
		}
		
		*res = base_url + url.substr(pos_b + option_name.length(), length);

	} else if (pos_s != std::string::npos) {

		*res = base_url + url.substr(pos_s + 1, url.size() - pos_s - 1);

	} else throw 0; // добавим как обычную ссылку

	return res;
}

} // end of namespace chatd
