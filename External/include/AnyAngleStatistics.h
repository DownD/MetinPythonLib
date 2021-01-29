/*
This code was developed by Tansel Uras (turas@usc.edu) at USC.
The code is hosted at 'http://idm-lab.org/anyangle'.
If you use this code in your research, please  cite our SoCS paper:

T. Uras and S. Koenig,  2015. An Empirical Comparison of Any-Angle Path-Planning Algorithms. In: Proceedings of the 8th Annual Symposium on Combinatorial
Search. Code available at: http://idm-lab.org/anyangle

Bibtex:
@inproceedings{uras:15,
  author = "T. Uras and S. Koenig",
  title = "An Empirical Comparison of Any-Angle Path-Planning Algorithms",
  booktitle = {Proceedings of the 8th Annual Symposium on Combinatorial Search},
  year = "2015",
  note = "Code available at: http://idm-lab.org/anyangle",
}
*/

#ifndef ANY_ANGLE_STATISTICS_H
#define ANY_ANGLE_STATISTICS_H

#include <string>
#include <iostream>
#include <iomanip>

#ifdef ANY_ANGLE_RUNNING_IN_HOG
#define ANY_ANGLE_VERBOSE
#endif

class AnyAngleStatistics {
public:
    AnyAngleStatistics(std::string algorithm_id)
    {
        num_instances_ = 0;
        total_search_time_ = 0;
        num_total_expansions_ = 0;
        num_total_generated_ = 0;
        num_total_percolations_ = 0;
        num_total_los_checks_ = 0;
        num_total_heading_changes_ = 0;
        num_total_freespace_heading_changes_ = 0;
        num_total_taut_corner_heading_changes_ = 0;
        num_total_non_taut_corner_heading_changes_ = 0;

        num_valid_instances_ = 0;
        total_valid_search_time_ = 0;
        total_valid_solution_cost_ = 0;
        num_valid_expansions_ = 0;
        num_valid_percolations_ = 0;
        num_valid_generated_ = 0;
        num_valid_los_checks_ = 0;
        num_valid_heading_changes_ = 0;
        num_valid_freespace_heading_changes_ = 0;
        num_valid_taut_corner_heading_changes_ = 0;
        num_valid_non_taut_corner_heading_changes_ = 0;


        algorithm_id_ = algorithm_id;
    }
    ~AnyAngleStatistics()
    {
    }

    // Opens up two files. 'summary' contains summary of the statistics with descriptive names. 'instances' contains the data for each reported instance.
    void OpenOutputFiles(std::string mapname)
    {
        if (mapname.empty() || algorithm_id_.empty())
            return;

        std::string detailed_filename = algorithm_id_ + "-" + mapname;
        summary_.open(detailed_filename.c_str());
        summary_ << std::fixed;

        std::string instances_filename = "Instances-" + algorithm_id_ + "-" + mapname;
        instances_.open(instances_filename.c_str());
        instances_ << std::fixed;
    }

    // Closes the output files.
    void CloseOutputFiles()
    {
        if (summary_.is_open())
            summary_.close();
        if (instances_.is_open()) {
            instances_ << std::endl;
            instances_.close();
        }
    }

    // Prints the value with the given description to the 'detailed' file and prints only the value to he 'brief' file.
    void ReportIntStatistic(const char * description, uint64_t val)
    {
        if (summary_.is_open())
            summary_ << description << ": " << val << std::endl;
#ifdef ANY_ANGLE_VERBOSE
        std::cout<<description<<": "<<val<<std::endl;
#endif
    }
    void ReportDoubleStatistic(const char * description, double val)
    {
        if (summary_.is_open())
            summary_ << description << ": " << val << std::endl;
#ifdef ANY_ANGLE_VERBOSE
        std::cout<<description<<": "<<val<<std::endl;
#endif
    }

    // Prints a sentence to the 'detailed' file, without any associated values.
    void AddRemark(std::string remark)
    {
        if (summary_.is_open())
            summary_ << remark << std::endl;
#ifdef ANY_ANGLE_VERBOSE
        std::cout<<remark<<std::endl;
#endif
    }

    // Prints all the accumulated statistics.
    void ReportAllStatistics()
    {
        ReportIntStatistic("Number of instances", num_instances_);
        ReportIntStatistic("Number of solved instances", num_valid_instances_);
        ReportIntStatistic("Number of unsolved instances", num_instances_ - num_valid_instances_);

        // Statistics for solved instances
        ReportDoubleStatistic("\nValid searches - Average solution cost", total_valid_solution_cost_ / num_valid_instances_);
        ReportDoubleStatistic("Valid searches - Average search time (ms)", (total_valid_search_time_ * 1000.0) / num_valid_instances_);
        ReportDoubleStatistic("Valid searches - Average number of expansions", ((double) num_valid_expansions_) / num_valid_instances_);
        ReportDoubleStatistic("Valid searches - Average number of states generated", ((double) num_valid_generated_) / num_valid_instances_);
        ReportDoubleStatistic("Valid searches - Average number of percolations", ((double) num_valid_percolations_) / num_valid_instances_);
        ReportDoubleStatistic("Valid searches - Average number of LOS checks", ((double) num_valid_los_checks_) / num_valid_instances_);
        ReportDoubleStatistic("Valid searches - Average number of heading changes", ((double) num_valid_heading_changes_) / num_valid_instances_);
        ReportDoubleStatistic("Valid searches - Average number of freespace heading changes",
                ((double) num_valid_freespace_heading_changes_) / num_valid_instances_);
        ReportDoubleStatistic("Valid searches - Average number of taut corner heading changes",
                ((double) num_valid_taut_corner_heading_changes_) / num_valid_instances_);
        ReportDoubleStatistic("Valid searches - Average number of non-taut corner heading changes",
                ((double) num_valid_non_taut_corner_heading_changes_) / num_valid_instances_);

        ReportDoubleStatistic("\nValid searches - Total solution cost", total_valid_solution_cost_); // Only use valid solution cost, otherwise solution cost can be infinite
        ReportDoubleStatistic("Valid searches - Total search time (ms)", total_valid_search_time_ * 1000.0);
        ReportIntStatistic("Valid searches - Total number of expansions", num_valid_expansions_);
        ReportIntStatistic("Valid searches - Total number of states generated", num_valid_generated_);
        ReportIntStatistic("Valid searches - Total number of percolations", num_valid_percolations_);
        ReportIntStatistic("Valid searches - Total number of LOS checks", num_valid_los_checks_);
        ReportIntStatistic("Valid searches - Total number of heading changes", num_valid_heading_changes_);
        ReportIntStatistic("Valid searches - Total number of freespace heading changes", num_valid_freespace_heading_changes_);
        ReportIntStatistic("Valid searches - Total number of taut corner heading changes", num_valid_taut_corner_heading_changes_);
        ReportIntStatistic("Valid searches - Total number of non-taut corner heading changes", num_valid_non_taut_corner_heading_changes_);

        ReportDoubleStatistic("\nAverage search time (ms)", (total_search_time_ * 1000.0) / num_instances_);
        ReportDoubleStatistic("Average number of expansions", ((double) num_total_expansions_) / num_instances_);
        ReportDoubleStatistic("Average number of states generated", ((double) num_total_generated_) / num_instances_);
        ReportDoubleStatistic("Average number of percolations", ((double) num_total_percolations_) / num_instances_);
        ReportDoubleStatistic("Average number of LOS checks", ((double) num_total_los_checks_) / num_instances_);
        ReportDoubleStatistic("Average number of heading changes", ((double) num_total_heading_changes_) / num_instances_);
        ReportDoubleStatistic("Average number of freespace heading changes", ((double) num_total_freespace_heading_changes_) / num_instances_);
        ReportDoubleStatistic("Average number of taut corner heading changes", ((double) num_total_taut_corner_heading_changes_) / num_instances_);
        ReportDoubleStatistic("Average number of non-taut corner heading changes", ((double) num_total_non_taut_corner_heading_changes_) / num_instances_);

        ReportDoubleStatistic("\nTotal search time (ms)", total_search_time_ * 1000.0);
        ReportIntStatistic("Total number of expansions", num_total_expansions_);
        ReportIntStatistic("Total number of states generated", num_total_generated_);
        ReportIntStatistic("Total number of percolations", num_total_percolations_);
        ReportIntStatistic("Total number of LOS checks", num_total_los_checks_);
        ReportIntStatistic("Total number of heading changes", num_total_heading_changes_);
        ReportIntStatistic("Total number of freespace heading changes", num_total_freespace_heading_changes_);
        ReportIntStatistic("Total number of taut corner heading changes", num_total_taut_corner_heading_changes_);
        ReportIntStatistic("Total number of non-taut corner heading changes", num_total_non_taut_corner_heading_changes_);

        CloseOutputFiles();
    }

    // Updates the statistics with data from a single search.
    void AddSearchData(
            double search_time, cost solution_cost, uint64_t num_expansions = 0, uint64_t num_generated = 0, uint64_t num_percolations = 0, uint64_t num_los_checks = 0,
            uint64_t num_heading_changes = 0, uint64_t num_freespace_heading_changes = 0,
            uint64_t num_taut_corner_heading_changes = 0, uint64_t num_non_taut_corner_heading_changes = 0)
    {
        num_instances_++;
        total_search_time_ += search_time;
        num_total_expansions_ += num_expansions;
        num_total_generated_ += num_generated;
        num_total_percolations_ += num_percolations;
        num_total_los_checks_ += num_los_checks;
        num_total_heading_changes_ += num_heading_changes;
        num_total_freespace_heading_changes_ += num_freespace_heading_changes;
        num_total_taut_corner_heading_changes_ += num_taut_corner_heading_changes;
        num_total_non_taut_corner_heading_changes_ += num_non_taut_corner_heading_changes;

        if (solution_cost != INFINITE_COST) {
            num_valid_instances_++;
            total_valid_search_time_ += search_time;
            total_valid_solution_cost_ += solution_cost;
            num_valid_expansions_ += num_expansions;
            num_valid_generated_ += num_generated;
            num_valid_percolations_ += num_percolations;
            num_valid_los_checks_ += num_los_checks;
            num_valid_heading_changes_ += num_heading_changes;
            num_valid_freespace_heading_changes_ += num_freespace_heading_changes;
            num_valid_taut_corner_heading_changes_ += num_taut_corner_heading_changes;
            num_valid_non_taut_corner_heading_changes_ += num_non_taut_corner_heading_changes;
        }

        if (instances_.is_open()) {
            instances_<<search_time*1000<<"\t";
            instances_<<solution_cost<<"\t";
            instances_<<num_expansions<<"\t";
            instances_<<num_generated<<"\t";
            instances_<<num_percolations<<"\t";
            instances_<<num_los_checks<<"\t";
            instances_<<num_heading_changes<<"\t";
            instances_<<num_freespace_heading_changes<<"\t";
            instances_<<num_taut_corner_heading_changes<<"\t";
            instances_<<num_non_taut_corner_heading_changes<<std::endl;
        }

#ifdef ANY_ANGLE_VERBOSE
        std::cout<<std::setw(12)<<algorithm_id_<<"\tTime: "<<search_time * 1000<<" ms.  \tCost: "<<solution_cost<<std::endl;
        // std::cout<<algorithm_id_<<std::endl;
        // std::cout<<"Search time:\t"<<search_time<<std::endl;
        // std::cout<<"Solution cost:\t"<<solution_cost<<std::endl;
        // std::cout<<"Expansions:\t"<<num_expansions<<std::endl;
        // std::cout<<"States generated:\t"<<num_generated<<std::endl;endl;
        // std::cout<<"Percolations:\t"<<num_percolations<<std::endl;
        // std::cout<<"LOS checks:\t"<<num_los_checks<<std::endl;
        // std::cout<<"Heading changes:\t"<<num_heading_changes<<std::endl;
        // std::cout<<"Freespace heading changes:\t"<<num_freespace_heading_changes<<std::endl;
        // std::cout<<"Taut corner heading changes:\t"<<num_taut_corner_heading_changes<<std::endl;
        // std::cout<<"Non-taut corner heading changes:\t"<<num_non_taut_corner_heading_changes<<std::endl;
        // std::cout<<std::endl;
#endif
    }

private:
    // 'total' variables sum up the values for all reported instances. 'valid' variables sum up the values for instances where a valid path is found.
    uint64_t num_instances_;
    double total_search_time_;
    uint64_t num_total_los_checks_, num_total_expansions_, num_total_generated_, num_total_percolations_;
    uint64_t num_total_heading_changes_, num_total_freespace_heading_changes_, num_total_taut_corner_heading_changes_, num_total_non_taut_corner_heading_changes_;

    uint64_t num_valid_instances_;
    cost total_valid_solution_cost_;
    double total_valid_search_time_;
    uint64_t num_valid_los_checks_, num_valid_expansions_, num_valid_generated_, num_valid_percolations_;
    uint64_t num_valid_heading_changes_, num_valid_freespace_heading_changes_, num_valid_taut_corner_heading_changes_, num_valid_non_taut_corner_heading_changes_;

    std::string algorithm_id_;
    std::ofstream summary_, instances_;
};

#endif
