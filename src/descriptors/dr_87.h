#ifndef _DVBPSI_DR_87_H_
#define _DVBPSI_DR_87_H_

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * dvbpsi_rating_dimension_s
 *****************************************************************************/
/*!
 * \struct dvbpsi_content_advisory_dr_s
 * \brief "rating dimension" descriptor structure.
 *
 * This structure is used to store a decoded "rating dimension"
 * descriptor.
 */
/*!
 * \typedef struct dvbpsi_rating_dimension_s dvbpsi_rating_dimension_t
 * \brief dvbpsi_rating_dimension_t type definition.
 */
typedef struct dvbpsi_rating_dimension_s
{
  uint8_t     i_rating_dimension;
  uint8_t     i_rating_value;

  struct dvbpsi_rating_dimension_s * p_next;

} dvbpsi_rating_dimension_t;


/*****************************************************************************
 * dvbpsi_rating_region_s
 *****************************************************************************/
/*!
 * \struct dvbpsi_content_advisory_dr_s
 * \brief "rating region" descriptor structure.
 *
 * This structure is used to store a decoded "rating region"
 * descriptor.
 */
/*!
 * \typedef struct dvbpsi_rating_dimension_s dvbpsi_rating_dimension_t
 * \brief dvbpsi_rating_dimension_t type definition.
 */
typedef struct dvbpsi_rating_region_s
{
  uint8_t          i_rating_region;

  multi_string_t * rating_description;

  struct dvbpsi_rating_dimension_s *p_first_dimension;

  struct dvbpsi_rating_region_s* p_next;

} dvbpsi_rating_region_t;

/*****************************************************************************
 * dvbpsi_content_advisory_dr_s
 *****************************************************************************/
/*!
 * \struct dvbpsi_content_advisory_dr_s
 * \brief "content advisory" descriptor structure.
 *
 * This structure is used to store a decoded "content advisory"
 * descriptor.
 */
/*!
 * \typedef struct dvbpsi_content_advisory_dr_s dvbpsi_content_advisory_dr_t
 * \brief dvbpsi_content_advisory_dr_t type definition.
 */
typedef struct dvbpsi_content_advisory_dr_s
{
	dvbpsi_rating_region_t * p_first_region;

} dvbpsi_content_advisory_dr_t;


/*****************************************************************************
 * dvbpsi_ContentAdvisoryAddRegion
 *****************************************************************************
 * Add a region in the CAD.
 *****************************************************************************/
dvbpsi_rating_region_t* dvbpsi_ContentAdvisoryDrAddRegion(dvbpsi_content_advisory_dr_t* p_decoded, dvbpsi_rating_region_t* p_region);



/*****************************************************************************
 * dvbpsi_RegionAddDimension
 *****************************************************************************
 * Add a dimension in the region.
 *****************************************************************************/
dvbpsi_rating_dimension_t* dvbpsi_RegionAddDimension(dvbpsi_rating_region_t* p_region, dvbpsi_rating_dimension_t* p_dimension);


/*****************************************************************************
 * dvbpsi_DecodeServiceDataDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_content_advisory_dr_t * dvbpsi_DecodeContentAdvisoryDr(
                                        dvbpsi_descriptor_t * p_descriptor)
 * \brief "content advisory" descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new "content advisory" descriptor structure
 * which contains the decoded data.
 */
dvbpsi_content_advisory_dr_t* dvbpsi_DecodeContentAdvisoryDr(
                                        dvbpsi_descriptor_t * p_descriptor);


/*****************************************************************************
 * dvbpsi_FreeServiceLocationDr
 *****************************************************************************/
/*!
 * \fn void  dvbpsi_FreeContentAdvisoryDr(
                                        dvbpsi_content_advisory_dr_t * p_descriptor)
 * \brief frees service location descriptor
 * \param p_descriptor pointer to the descriptor structure
 */
void dvbpsi_FreeContentAdvisoryDr(dvbpsi_content_advisory_dr_t * p_descriptor);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dr_87.h"
#endif
