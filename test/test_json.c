#include "../lhttpd.h"
#include <assert.h>

int main(int argc, char *argv[])
{
#if HAS_JSON_C
    json_object *jobj;
    const char *jstr;

    const char *key1 = "world";
    const char *key2 = "I am foo";
    const char *key3;
    l_bool_t bool2;

    l_json_map_t maps1[] = {
        { &key1, "hello", json_type_string },
        L_JSON_MAP_END
    };
    l_json_map_t maps2[] = {
        { &key2, "foo", json_type_string },
        L_JSON_MAP_END
    };
    // `type` doesn't matter
    l_json_map_t maps3[] = {
        { &bool2, "two", 0 },
        { &key3, "three", 0 },
        L_JSON_MAP_END
    };

    l_json_map_t maps4[2] = {};

    jstr = l_json_dumps(maps1);
    assert(!strcmp(jstr, "{ \"hello\": \"world\" }"));
    L_FREE(jstr);

    jobj = l_json_dump(maps2);
    // replace `{"foo": "I am foo"}` to `{"foo": "Not foo"}`
    l_json_add_string(jobj, "foo", "Not foo");
    l_json_add_int(jobj, "bar", 8);
    assert(!strcmp(l_json_to_string(jobj), "{ \"foo\": \"Not foo\", \"bar\": 8 }"));
    l_free_json(jobj);

    jobj = l_create_json_object();
    l_json_add_bool(jobj, "two", FALSE);
    l_json_add_string(jobj, "three", "four");

    l_json_load(maps3, jobj);
    assert(bool2 == FALSE && !strcmp(key3, "four"));

    json_object *array = l_create_json_array();
    l_array_add_string(array, "one_simple_array");
    l_array_add_int(array, 1);
    l_array_add_jobj(array, jobj);
    jstr = "[ \"one_simple_array\", 1, { \"two\": false, \"three\": \"four\" } ]";
    assert(!strcmp(l_json_to_string(array), jstr));
    l_free_json(jobj);

    json_object *tmp = l_json_loads(maps4, jstr, strlen(jstr));
    key1 = l_json_to_string(maps4[0].var);
    key2 = l_json_to_string(maps4[1].var);
    assert(!strcmp(key1, "\"one_simple_array\""));
    assert(!strcmp(key2, "1"));
    l_free_json(tmp);
    l_free_json(array);
#endif
    return 0;
}
