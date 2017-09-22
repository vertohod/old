#include "gtest/gtest.h"

#include <string>

#include "singleton.h"
#include "blizzard.h"
#include "message.h"
#include "image.h"
#include "audio.h"

TEST (BlizzardTest, CheckAddTable)
{
	size_t server_number = 1;
	size_t max_number = 1;

	blizzard::blizzard bl(server_number, max_number);

	bl.add_table<chatd::image>();

	bool flag_ok = true;

	try {
		auto tb = bl.table(chatd::image::name());
	} catch (...) {
		flag_ok = false;
	}

	ASSERT_TRUE(flag_ok);
}

TEST (BlizzardTest, CheckAddObjectIntoTable)
{
	size_t server_number = 1;
	size_t max_number = 1;

	blizzard::blizzard bl(server_number, max_number);

	bl.add_table<chatd::image>();

	auto image_ptr = chatd::image::create();
	image_ptr->set_flag_test();

	ASSERT_GT(bl.insert(image_ptr), 0);
}

TEST (BlizzardTest, CheckSearchingObjectInTable)
{
	size_t server_number = 1;
	size_t max_number = 1;

	blizzard::blizzard bl(server_number, max_number);

	bl.add_table<chatd::image>();

	auto image_ptr = chatd::image::create();
	image_ptr->set_flag_test();
	auto new_object_oid = bl.insert(image_ptr);

	bool flag_ok = true;
	try {
		auto ptr = bl.get<chatd::image>(new_object_oid);
	} catch (...) {
		flag_ok = false;
	}

	ASSERT_TRUE(flag_ok);
}

TEST (BlizzardTest, CheckObjectSerialization)
{
	size_t server_number = 1;
	size_t max_number = 1;

	blizzard::blizzard bl(server_number, max_number);

	bl.add_table<chatd::image>();

	auto image_ptr = chatd::image::create();
	image_ptr->set_flag_test();
	image_ptr->set_file_name("some_name.gif");
	image_ptr->set_content_type("image");
	image_ptr->set_path_temporary("/tmp/");
	image_ptr->set_extension("gif");
	auto new_object_oid = bl.insert(image_ptr);

	auto ptr = bl.get<chatd::image>(new_object_oid);
	auto json = ptr.lock()->serialization();

	ASSERT_TRUE(!json->empty());
}

TEST (BlizzardTest, CheckObjectUpdate)
{
	size_t server_number = 1;
	size_t max_number = 1;

	blizzard::blizzard bl(server_number, max_number);

	bl.add_table<chatd::image>();

	OID new_object_oid1 = 0;
	{ // image 1
		auto image_ptr = chatd::image::create();
		image_ptr->set_flag_test();
		image_ptr->set_file_name("some_name.png");
		image_ptr->set_content_type("image");
		image_ptr->set_path_temporary("/tmp/");
		image_ptr->set_extension("png");
		new_object_oid1 = bl.insert(image_ptr);
	}

	OID new_object_oid2 = 0;
	{ // image 2
		auto image_ptr = chatd::image::create();
		image_ptr->set_flag_test();
		image_ptr->set_file_name("some_name.jpg");
		image_ptr->set_content_type("image");
		image_ptr->set_path_temporary("/tmp/");
		image_ptr->set_extension("jpg");
		new_object_oid2 = bl.insert(image_ptr);
	}

	auto ptr1 = bl.get<chatd::image>(new_object_oid1);
	std::cout << *(ptr1.lock()->serialization()) << std::endl;

	auto ptr2 = bl.get<chatd::image>(new_object_oid2);
	std::cout << *(ptr2.lock()->serialization()) << std::endl;

	{ // image 2.2
		auto image_ptr_copy = ptr2.lock()->make_copy();
		image_ptr_copy->set_owner(2);
		dynamic_cast<chatd::image*>(image_ptr_copy.get())->set_flag_test();
		dynamic_cast<chatd::image*>(image_ptr_copy.get())->set_file_name("new_file_name.jpg");
		bl.update(image_ptr_copy);
	}

	auto ptr3 = bl.get<chatd::image>(new_object_oid2);
	std::cout << *(ptr3.lock()->serialization()) << std::endl;

	ASSERT_TRUE(bl.table(chatd::image::name())->size() == 2);
}

TEST (BlizzardTest, CheckFindOwnerKey)
{
	size_t server_number = 1;
	size_t max_number = 1;

	blizzard::blizzard bl(server_number, max_number);

	bl.add_table<chatd::image>();

	auto image_ptr1 = chatd::image::create();
	auto image_ptr2 = chatd::image::create();
	auto image_ptr3 = chatd::image::create();
	auto image_ptr4 = chatd::image::create();
	image_ptr1->set_flag_test();
	image_ptr2->set_flag_test();
	image_ptr3->set_flag_test();
	image_ptr4->set_flag_test();

	image_ptr1->set_owner(5);
	image_ptr2->set_owner(5);
	image_ptr3->set_owner(7);
	image_ptr4->set_owner(5);

	bl.insert(image_ptr1);
	bl.insert(image_ptr2);
	bl.insert(image_ptr3);
	bl.insert(image_ptr4);

	auto key = std::make_shared<blizzard::object::key_owner>();
	key->set_owner(5);

	auto res = bl.find<chatd::image>(key);

	ASSERT_EQ(res->size(), 3);
}

#define ITERATIONS 10000

TEST (BlizzardTest, CheckManyInsert)
{
	size_t server_number = 1;
	size_t max_number = 1;

	blizzard::blizzard bl(server_number, max_number);

	bl.add_table<chatd::image>();

	for (size_t i = 0; i < ITERATIONS; ++i) {
		auto image_ptr = chatd::image::create();
		image_ptr->set_flag_test();
		image_ptr->set_owner(i % 100);
		auto oid = bl.insert(image_ptr);

		ASSERT_NE(oid, 0);
	}

	ASSERT_EQ(bl.table(chatd::image::name())->size(), ITERATIONS);

	auto key = std::make_shared<blizzard::object::key_owner>();
	key->set_owner(5);

	auto res = bl.find<chatd::image>(key);

	ASSERT_EQ(res->size(), ITERATIONS / 100);
}

TEST (BlizzardTest, CheckManyUpdate)
{
	size_t server_number = 1;
	size_t max_number = 1;

	blizzard::blizzard bl(server_number, max_number);

	bl.add_table<chatd::image>();

	for (size_t i = 0; i < ITERATIONS; ++i) {
		auto image_ptr = chatd::image::create();
		image_ptr->set_flag_test();
		image_ptr->set_owner(i % 100);
		auto oid = bl.insert(image_ptr);

		ASSERT_NE(oid, 0) << "i = " << i;

		auto ptr = bl.get<chatd::image>(oid);
		auto ptr_copy = ptr.lock()->make_copy();
		dynamic_cast<chatd::image*>(ptr_copy.get())->set_flag_test();
		dynamic_cast<chatd::image*>(ptr_copy.get())->set_owner((i + 3) % 100);

		ASSERT_TRUE(bl.update(ptr_copy));
	}

	ASSERT_EQ(bl.table(chatd::image::name())->size(), ITERATIONS);

	auto key = std::make_shared<blizzard::object::key_owner>();
	key->set_owner(5);

	auto res = bl.find<chatd::image>(key);

	ASSERT_EQ(res->size(), ITERATIONS / 100);
}

TEST (BlizzardTest, CheckInsertingMessageObject)
{
	size_t server_number = 1;
	size_t max_number = 1;

	singleton<blizzard::blizzard>::init(new blizzard::blizzard(server_number, max_number));

	bl().add_table<chatd::message>();

	auto ptr = chatd::message::create();
	auto images = std::make_shared<std::vector<OID>>();
	images->push_back(1);
	images->push_back(2);
	images->push_back(3);
	ptr->set_images(images);

	auto oid = bl().insert(ptr);

	ASSERT_GT(oid, 0);

	auto mess_ptr = bl().get<chatd::message>(oid);
	std::cout << *(mess_ptr.lock()->serialization()) << std::endl;
}

TEST (BlizzardTest, CheckMessageDeserialization)
{
	size_t server_number = 1;
	size_t max_number = 1;

	blizzard::blizzard bl(server_number, max_number);

	bl.add_table<chatd::message>();

	std::string line("event=0\ttime=0\tindex=1099511627779\towner=0\tname=message\tversion=1\tpoid=0\toid_from=0\toid_to=0\tprivate=false\ttime=0\tcolor=0\tavatar=\tname_from=\tname_to=\tmessage=\traw=\timages=1,2,3\tyoutube=\t");
	auto ptr = chatd::message::create();
	ptr->deserialization(line);
	auto json = ptr->serialization();

	ASSERT_EQ(line.size(), json->size()) << line << std::endl << *json;
}

TEST (BlizzardTest, CheckAddAudioIntoTable)
{
	size_t server_number = 1;
	size_t max_number = 1;

	blizzard::blizzard bl(server_number, max_number);

	bl.add_table<chatd::audio>();

	auto ptr = chatd::audio::create();

	ASSERT_GT(bl.insert(ptr), 0);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
