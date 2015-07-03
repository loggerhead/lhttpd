#include "json.h"

static void _load_json_object(l_json_map_t maps[], json_object *jobj)
{
    struct json_object_iterator begin = json_object_iter_begin(jobj);
    struct json_object_iterator end   = json_object_iter_end(jobj);

    for (; !json_object_iter_equal(&begin, &end); json_object_iter_next(&begin)) {
        const char *key = json_object_iter_peek_name(&begin);
        json_object *val = json_object_iter_peek_value(&begin);

        for (int i = 0; maps[i].key; i++) {
            l_json_map_t map = maps[i];

            if (strcmp(map.key, key))
                continue;

            switch (json_object_get_type(val)) {
                case json_type_boolean:
                    *(l_bool_t *)    map.var = json_object_get_boolean(val);
                    break;
                case json_type_double:
                    *(double *)      map.var = json_object_get_double(val);
                    break;
                case json_type_int:
                    *(int *)         map.var = json_object_get_int(val);
                    break;
                case json_type_string:
                    *(const char **) map.var = json_object_get_string(val);
                    break;
                case json_type_object:
                    *(json_object **)map.var = val;
                    break;
                case json_type_array:
                    *(array_list **) map.var = json_object_get_array(val);
                    break;
                default:
                    l_warn("%s: unknown json type", __func__);
                    break;
            }
            break;
        }
    }
}

static void _load_json_array(l_json_map_t maps[], json_object *jobj)
{
    int len = json_object_array_length(jobj);
    for (int i = 0; i < len; i++) {
        maps[i].var = json_object_array_get_idx(jobj, i);
    }
}

void l_json_load(l_json_map_t maps[], json_object *jobj)
{
    switch (json_object_get_type(jobj)) {
        case json_type_boolean:
            *(l_bool_t *)    maps[0].var = json_object_get_boolean(jobj);
            break;
        case json_type_double:
            *(double *)      maps[0].var = json_object_get_double(jobj);
            break;
        case json_type_int:
            *(int *)         maps[0].var = json_object_get_int(jobj);
            break;
        case json_type_string:
            *(const char **) maps[0].var = json_object_get_string(jobj);
            break;
        case json_type_object:
            _load_json_object(maps, jobj);
            break;
        case json_type_array:
            _load_json_array(maps, jobj);
            break;
        default:
            l_warn("%s: unknown json type", __func__);
            break;
    }
}

// NOTE: result need free by `l_json_free` if no more need of `maps` variable
json_object *l_json_loads(l_json_map_t maps[], const char *jstr, size_t len)
{
    json_tokener *tok = json_tokener_new();
    json_object *jobj = json_tokener_parse_ex(tok, jstr, len);

    if (json_tokener_get_error(tok) == json_tokener_success) {
        l_json_load(maps, jobj);
    } else {
        l_warn("%s: json parse failed", __func__);
    }

    json_tokener_free(tok);
    return jobj;
}

// NOTE: result need free by `l_json_free`
json_object *l_json_dump(l_json_map_t maps[])
{
    json_object *jobj = json_object_new_object();

    for (int i = 0; maps[i].key; i++) {
        l_json_map_t map = maps[i];
        json_object *val = NULL;

        switch (map.type) {
            case json_type_boolean:
                val = json_object_new_boolean(*(l_bool_t *)map.var);
                break;
            case json_type_double:
                val = json_object_new_double(*(double *)map.var);
                break;
            case json_type_int:
                val = json_object_new_int(*(int *)map.var);
                break;
            case json_type_string:
                val = json_object_new_string(*(const char **)map.var);
                break;
            case json_type_object: // fall though
            case json_type_array:
                val = *(json_object **)map.var;
                break;
            default:
                l_warn("%s: unknown json type", __func__);
                break;
        }

        json_object_object_add(jobj, map.key, val);
    }

    return jobj;
}

// NOTE: return value need free
const char *l_json_dumps(l_json_map_t maps[])
{
    json_object *jobj = l_json_dump(maps);
    const char *jstr = strdup(json_object_to_json_string(jobj));
    l_free_json(jobj);
    return jstr;
}


json_object *l_create_json_object()
{
    return json_object_new_object();
}

json_object *l_create_json_array()
{
    return json_object_new_array();
}

void l_free_json(json_object *jobj)
{
    json_object_put(jobj);
}

const char *l_json_to_string(json_object *jobj)
{
    return json_object_to_json_string(jobj);
}


void l_json_add_string(json_object *jobj, const char *key, const char *val)
{
    json_object_object_add(jobj, key, json_object_new_string(val));
}

void l_json_add_double(json_object *jobj, const char *key, double val)
{
    json_object_object_add(jobj, key, json_object_new_double(val));
}

void l_json_add_int(json_object *jobj, const char *key, int val)
{
    json_object_object_add(jobj, key, json_object_new_int(val));
}

void l_json_add_bool(json_object *jobj, const char *key, l_bool_t val)
{
    json_object_object_add(jobj, key, json_object_new_boolean(val));
}

void l_json_add_jobj(json_object *jobj, const char *key, json_object *val)
{
    json_object_object_add(jobj, key, val);
}


void l_array_add_string(json_object *jobj, const char *val)
{
    json_object_array_add(jobj, json_object_new_string(val));
}

void l_array_add_double(json_object *jobj, double val)
{
    json_object_array_add(jobj, json_object_new_double(val));
}

void l_array_add_int(json_object *jobj, int val)
{
    json_object_array_add(jobj, json_object_new_int(val));
}

void l_array_add_bool(json_object *jobj, l_bool_t val)
{
    json_object_array_add(jobj, json_object_new_boolean(val));
}

void l_array_add_jobj(json_object *jobj, json_object *val)
{
    json_object_array_add(jobj, val);
}