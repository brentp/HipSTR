#ifndef SNP_BAM_PROCESSOR_H_
#define SNP_BAM_PROCESSOR_H_

#include <iostream>
#include <string>
#include <vector>

#include "bamtools/include/api/BamAlignment.h"
#include "vcflib/src/Variant.h"

#include "bam_processor.h"
#include "base_quality.h"
#include "error.h"
#include "region.h"


const std::string HAPLOTYPE_TAG = "HP";
const double FROM_HAP_LL        = -0.01;   // Log-likelihood read comes from a haplotype if it matches BAM HP tag
const double OTHER_HAP_LL       = -1000.0; // Log-likelihood read comes from a haplotype if it differs from BAM HP tag

class SNPBamProcessor : public BamProcessor {
private:
  bool have_snp_vcf_;
  vcflib::VariantCallFile phased_snp_vcf_;
  int32_t match_count_, mismatch_count_;

  // Timing statistics (in seconds)
  double total_snp_phase_info_time_;
  double locus_snp_phase_info_time_;

  bool bams_from_10x_; // True iff BAMs were generated from 10X GEMCODE platform

  // Process reads from BAM generated by 10X genomics
  // Requires HP tag, which indicates which haplotype reads came from
  void process_10x_reads(std::vector< std::vector<BamTools::BamAlignment> >& paired_strs_by_rg,
			 std::vector< std::vector<BamTools::BamAlignment> >& mate_pairs_by_rg,
			 std::vector< std::vector<BamTools::BamAlignment> >& unpaired_strs_by_rg,
			 std::vector<std::string>& rg_names, Region& region, std::string& ref_allele, std::string& chrom_seq,
			 std::ostream& out);

  // Extract the haplotype for an alignment based on the HP tag
  int get_haplotype(BamTools::BamAlignment& aln);


public:
 SNPBamProcessor(bool use_bam_rgs, bool remove_pcr_dups):BamProcessor(use_bam_rgs, remove_pcr_dups){
    have_snp_vcf_    = false;
    match_count_     = 0;
    mismatch_count_  = 0;
    total_snp_phase_info_time_  = 0;
    locus_snp_phase_info_time_  = -1;
    bams_from_10x_              = false;
  }

  double total_snp_phase_info_time() { return total_snp_phase_info_time_; }
  double locus_snp_phase_info_time() { return locus_snp_phase_info_time_; }

  void process_reads(std::vector< std::vector<BamTools::BamAlignment> >& paired_strs_by_rg,
		     std::vector< std::vector<BamTools::BamAlignment> >& mate_pairs_by_rg,
		     std::vector< std::vector<BamTools::BamAlignment> >& unpaired_strs_by_rg,
		     std::vector<std::string>& rg_names, Region& region, std::string& ref_allele, std::string& chrom_seq,
		     std::ostream& out);

  virtual void analyze_reads_and_phasing(std::vector< std::vector<BamTools::BamAlignment> >& alignments,
					 std::vector< std::vector<double> >& log_p1s, 
					 std::vector< std::vector<double> >& log_p2s,
					 std::vector<std::string>& rg_names, Region& region, std::string& ref_allele, std::string& chrom_seq, int iter){
    log("Ignoring read phasing probabilties");
  }

  void use_10x_bam_tags(){
    bams_from_10x_ = true;
  }

  void set_input_snp_vcf(std::string& vcf_file){
    if(!phased_snp_vcf_.open(vcf_file))
      printErrorAndDie("Failed to open input SNP VCF file");
    have_snp_vcf_ = true;
  }

  void finish(){
    log("SNP matching statistics: " + std::to_string(match_count_) + "\t" + std::to_string(mismatch_count_));
  }
};


#endif
