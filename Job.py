     #!/usr/bin/python


class Job(object):

    def __init__(self, job_id, submit_time, requested_time, actual_time, number_of_procs, run_queue_start, run_queue_end, sys_procs_avail_at_run_queue_start, is_dag_job):
        object.__init__(self)
        self.job_id = job_id
        self.submit_time = submit_time
        self.requested_time = requested_time
        self.actual_time = actual_time
        self.number_of_procs = number_of_procs
        self.run_queue_start = run_queue_start 
        self.run_queue_end = run_queue_end
        self.sys_procs_avail_at_run_queue_start = sys_procs_avail_at_run_queue_start
        self.is_dag_job = is_dag_job

    def __str__(self):
        string = str(self.job_id)
        string += "-" + str(self.actual_time)
        return string
    
    def __eq__(self,other):
        if self.job_id != other.job_id:
            return False
        if self.requested_time != other.submit_time:
            return False
        if self.actual_time != other.actual_time:
            return False
        return True
