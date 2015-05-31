#include "json_serializer.h"

#include <jansson.h>
#include <assert.h>

static int json_serializer_writeObject(dyn_type *type, json_t *object, void **result);
static void json_serializer_writeObjectMember(dyn_type *type, const char *name, json_t *val, void *inst); 
static void json_serializer_writeSequence(json_t *array, dyn_type *seq, void *seqLoc); 

int json_deserialize(dyn_type *type, const char *input, void **result) {
    //FIXME function is assuming complex type
    int status = 0;


    json_error_t error;
    json_t *root = json_loads(input, JSON_DECODE_ANY, &error);

    if (root != NULL) {
        status = json_serializer_writeObject(type, root, result);
        json_decref(root);
    } else {
        status = 1;
        printf("JSON_SERIALIZER: error parsing json input '%s'. Error is %s\n", input, error.text);
    }

    return status;
}

static int json_serializer_writeObject(dyn_type *type, json_t *object, void **result) {
    assert(object != NULL);
    int status = 0;

    void *inst = dynType_alloc(type);
    json_t *value;
    const char *key;

    if (inst != NULL)  {
        json_object_foreach(object, key, value) {
            json_serializer_writeObjectMember(type, key, value, inst);
        }
        *result = inst;
    } else {
        status = 1;
        printf("JSON_SERIALIZER: Error allocating memory\n");
    }

    return status;
}
                
static void json_serializer_writeObjectMember(dyn_type *type, const char *name, json_t *val, void *inst) {
    //TODO rename to complex. write generic write
    int index = dynType_complex_index_for_name(type, name);
    char charType = dynType_complex_schemaType_at(type, index); 
    void *valp = dynType_complex_val_loc_at(type, inst, index);


    float *f;
    double *d;
    char *c;
    short *s;
    int *i;
    long *l;
    dyn_type *nested;

    switch (charType) {
        case 'F' :
            f = valp;
            *f = json_real_value(val);
            break;
        case 'D' :
            d = valp;
            *d = json_real_value(val);
            break;
        case 'B' :
            c = valp;
            *c = json_integer_value(val);
            break;
        case 'S' :
            s = valp;
            *s = json_integer_value(val);
            break;
        case 'I' :
            i = valp;
            *i = json_integer_value(val);
            break;
        case 'J' :
            l = valp;
            *l = json_integer_value(val);
            break;
        case '[' :
            nested = dynType_complex_dynType_at(type, index); 
            json_serializer_writeSequence(val, nested, valp); 
            break;
        case '{' :
            nested = dynType_complex_dynType_at(type, index); 
            json_serializer_writeObject(nested, val, (void **)valp); 
            break;
        default :
            printf("JSON_SERIALIZER: error provided type '%c' not supported\n", charType);
            printf("Skipping\n");
    }
} 

static void json_serializer_writeSequence(json_t *array, dyn_type *seq, void *seqLoc) {
    assert(dynType_type(seq) == DYN_TYPE_SEQUENCE);
    size_t size = json_array_size(array);
    //char seqType = dynType_sequence_elementSchemaType(seq);

    dynType_sequence_init(seq, seqLoc, size); //seq is already allocated. only need to allocate the buf

    //assuming int
    int32_t i;
    int index;
    json_t *val;
    json_array_foreach(array, index, val) {
        i = json_number_value(val);
        dynType_sequence_append(seq, seqLoc, &i);
    }
}

int json_serialize(dyn_type *type, void *input, char **output, size_t *size) {
    return 0;
}
