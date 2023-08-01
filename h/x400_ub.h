/* x400_ub.h: X.400 upper bounds on things */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/x400_ub.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: x400_ub.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */

#define UB_INTEGER_OPTIONS  				256
#define UB_QUEUE_SIZE  					2147483647
#define UB_CONTENT_LENGTH  				2147483647
#define UB_PASSWORD_LENGTH  				62
#define UB_BIT_OPTIONS  				16
#define UB_CONTENT_TYPES  				1024
#define UB_TSAP_ID_LENGTH  				16
#define UB_RECIPIENTS  					32767
#define UB_CONTENT_ID_LENGTH  				16
#define UB_X121_ADDRESS_LENGTH  			15
#define UB_MTS_USER_TYPES  				256
#define UB_REASON_CODES  				32767
#define UB_DIAGNOSTIC_CODES  				32767
#define UB_SUPPLEMENTARY_INFO_LENGTH  			256
#define UB_EXTENSION_TYPES  				256
#define UB_RECIPIENT_NUMBER_FOR_ADVICE_LENGTH  		32
#define UB_CONTENT_CORRELATOR_LENGTH  			512
#define UB_REDIRECTIONS  				512
#define UB_DL_EXPANSIONS  				512
#define UB_BUILT_IN_CONTENT_TYPE  			32767
#define UB_LOCAL_ID_LENGTH  				32
#define UB_MTA_NAME_LENGTH  				32
#define UB_COUNTRY_NAME_NUMERIC_LENGTH  		3
#define UB_COUNTRY_NAME_ALPHA_LENGTH  			2
#define UB_DOMAIN_NAME_LENGTH  				16
#define UB_TERMINAL_ID_LENGTH  				24
#define UB_ORGANIZATION_NAME_LENGTH  			64
#define UB_NUMERIC_USER_ID_LENGTH  			32
#define UB_SURNAME_LENGTH  				40
#define UB_GIVEN_NAME_LENGTH  				16
#define UB_INITIALS_LENGTH  				5
#define UB_GENERATION_QUALIFIER_LENGTH  		3
#define UB_ORGANIZATIONAL_UNITS  			4
#define UB_ORGANIZATIONAL_UNIT_NAME_LENGTH  		32
#define UB_DOMAIN_DEFINED_ATTRIBUTES  			4
#define UB_DOMAIN_DEFINED_ATTRIBUTE_TYPE_LENGTH  	8
#define UB_DOMAIN_DEFINED_ATTRIBUTE_VALUE_LENGTH  	128
#define UB_EXTENSION_ATTRIBUTES  			256
#define UB_COMMON_NAME_LENGTH  				64
#define UB_PDS_NAME_LENGTH  				16
#define UB_POSTAL_CODE_LENGTH  				16
#define UB_PDS_PARAMETER_LENGTH  			30
#define UB_PDS_PHYSICAL_ADDRESS_LINES  			6
#define UB_UNFORMATTED_ADDRESS_LENGTH  			180
#define UB_E163_4_NUMBER_LENGTH  			15
#define UB_E163_4_SUB_ADDRESS_LENGTH  			40
#define UB_BUILT_IN_ENCODED_INFORMATION_TYPES  		32
#define UB_TELETEX_PRIVATE_USE_LENGTH  			128
#define UB_ENCODED_INFORMATION_TYPES  			1024
#define UB_SECURITY_LABELS  				256
#define UB_LABELS_AND_REDIRECTIONS  			256
#define UB_SECURITY_PROBLEMS  				256
#define UB_PRIVACY_MARK_LENGTH  			128
#define UB_SECURITY_CATEGORIES  			64
#define UB_TRANSFERS  					512
#define UB_BILATERAL_INFO  				1024
#define UB_ADDITIONAL_INFO  				1024