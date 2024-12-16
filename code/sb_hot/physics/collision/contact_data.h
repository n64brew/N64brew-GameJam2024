#ifndef CONTACT_DATA
#define CONTACT_DATA

#define MAX_CONTACTS 3

typedef struct
{
    Vector3 point;
    Vector3 normal;
    float penetration;
} ContactData;

void contactData_init(ContactData *contact)
{
    contact->point = (Vector3){0, 0, 0};
    contact->normal = (Vector3){0, 0, 0};
    contact->penetration = 0.0f;
}

#endif