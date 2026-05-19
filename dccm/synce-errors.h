#ifndef SYNCE_ERRORS_H
#define SYNCE_ERRORS_H

#include <glib-object.h>

G_BEGIN_DECLS

typedef enum
{
  InvalidArgument,
  NotAvailable,
} SynceErrors;

GQuark synce_errors_quark (void);
#define SYNCE_ERRORS synce_errors_quark ()

G_END_DECLS

#endif /* SYNCE_ERRORS_H */
