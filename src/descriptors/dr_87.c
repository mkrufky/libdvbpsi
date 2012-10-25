#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#include "../dvbpsi.h"
#include "../dvbpsi_private.h"
#include "../descriptor.h"
#include "../multi_string.h"


#include "dr_87.h"

/*****************************************************************************
 * dvbpsi_ContentAdvisoryAddRegion
 *****************************************************************************
 * Add a region in the CAD.
 *****************************************************************************/
dvbpsi_rating_region_t* dvbpsi_ContentAdvisoryDrAddRegion(dvbpsi_content_advisory_dr_t* p_decoded, dvbpsi_rating_region_t* p_region)
{
    if (p_decoded->p_first_region == NULL)
      p_decoded->p_first_region = p_region;
    else
    {
      dvbpsi_rating_region_t* p_last_element = p_decoded->p_first_region;
      while (p_last_element->p_next != NULL)
          p_last_element = p_last_element->p_next;
        p_last_element->p_next = p_region;
    }

    return p_region;
}

/*****************************************************************************
 * dvbpsi_RegionAddDimension
 *****************************************************************************
 * Add a dimension in the region.
 *****************************************************************************/
dvbpsi_rating_dimension_t* dvbpsi_RegionAddDimension(dvbpsi_rating_region_t* p_region, dvbpsi_rating_dimension_t* p_dimension)
{
    if (p_region->p_first_dimension == NULL)
      p_region->p_first_dimension = p_dimension;
    else
    {
      dvbpsi_rating_dimension_t* p_last_element = p_region->p_first_dimension ;
      while (p_last_element->p_next != NULL)
          p_last_element = p_last_element->p_next;
        p_last_element->p_next = p_dimension;
    }

    return p_dimension;
}

/*****************************************************************************
 * dvbpsi_DecodeContentAdvisoryDr
 *****************************************************************************/
dvbpsi_content_advisory_dr_t * dvbpsi_DecodeContentAdvisoryDr(
                                        dvbpsi_descriptor_t * p_descriptor)
{
  dvbpsi_content_advisory_dr_t * p_decoded;
  int i;
  uint8_t * p_data;
  uint8_t rating_region_count = 0;

  /* Check the tag */
  if(p_descriptor->i_tag != 0x87)
    return NULL;

  /* Don't decode twice */
  if(p_descriptor->p_decoded)
    return p_descriptor->p_decoded;

  /* Allocate memory */
  p_decoded =
        (dvbpsi_content_advisory_dr_t*)calloc(1, sizeof(dvbpsi_content_advisory_dr_t));
  if(!p_decoded) return NULL;

  /* Decode data and check the length */
  if(p_descriptor->i_length < 3)
  {
    free(p_decoded);
    return NULL;
  }

  p_descriptor->p_decoded = (void*)p_decoded;

  rating_region_count = p_descriptor->p_data[0] & 0x3f;

  p_data = &p_descriptor->p_data[1];

  for (i=0; i<rating_region_count; i++)
  {
	  uint8_t rating_region = p_data[0];
	  uint8_t rated_dimensions = p_data[1];
	  uint8_t rating_description_length;
	  dvbpsi_rating_region_t *p_region = NULL;
	  int j;

	  p_region = (dvbpsi_rating_region_t*)calloc(1, sizeof(dvbpsi_rating_region_t));

	  if (p_region == NULL) return NULL;

	  p_data = &p_data[2];

	  for (j=0; j<rated_dimensions; j++)
	  {
		  dvbpsi_rating_dimension_t *p_demension = NULL;

		  p_demension = (dvbpsi_rating_dimension_t*)calloc(1, sizeof(dvbpsi_rating_dimension_t));
		  if (p_demension == NULL) return NULL;

		  p_demension->i_rating_dimension = p_data[0];
		  p_demension->i_rating_value = p_data[1] & 0xf;

		  dvbpsi_RegionAddDimension(p_region, p_demension);

		  p_data += 2;
	  }

	  rating_description_length = p_data[0];

	  p_region->i_rating_region = rating_region;
	  if (rating_description_length != 0)
		  p_region->rating_description = dvbpsi_BuildMultiString(&p_data[1]);

	  dvbpsi_ContentAdvisoryDrAddRegion(p_decoded, p_region);

	  p_data += 1 + rating_description_length;
  }

  return p_decoded;
}

/*****************************************************************************
 * dvbpsi_FreeServiceLocationDr
 *****************************************************************************/
void dvbpsi_FreeContentAdvisoryDr(dvbpsi_content_advisory_dr_t * descriptor)
{
  dvbpsi_rating_region_t* p_region = NULL;

 if (descriptor == NULL)
   return;

 p_region = descriptor->p_first_region;

 while(p_region)
 {
   dvbpsi_rating_region_t* p_next = p_region->p_next;
   dvbpsi_rating_dimension_t *p_dimension = p_region->p_first_dimension;

   while(p_dimension)
   {
	   dvbpsi_rating_dimension_t *p_next_dimension = p_dimension->p_next;
	   free(p_dimension);
	   p_dimension = p_next_dimension;
   }
   free(p_region);
   p_region = p_next;
 }
}
