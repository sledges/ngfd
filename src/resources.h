#ifndef RESOURCES_H
#define RESOURCES_H

#define RESOURCE_BIT(bit) (1 << bit)

#define RESOURCE_AUDIO      RESOURCE_BIT(0)
#define RESOURCE_VIBRATION  RESOURCE_BIT(1)
#define RESOURCE_LEDS       RESOURCE_BIT(2)
#define RESOURCE_BACKLIGHT  RESOURCE_BIT(3)

#define SET_RESOURCE(value, resource) \
    { value |= resource; }

#define RESOURCE_ENABLED(value, resource) \
    (value & resource)

#endif /* RESOURCES_H */
