#!/bin/bash

# $1 - domain
# $2 - problem
# $3 - q
# $4 - k bound

NEW_UUID=$(uuidgen | tr "[:upper:]" "[:lower:]")
PLANFOLDER="/tmp/planner_runs/$NEW_UUID/found_plans"

echo $PLANFOLDER
mkdir -p $PLANFOLDER

./fast-downward.py --plan-file "$PLANFOLDER/sas_plan" $1 $2 --search "symq-bd(plan_selection=top_k(num_plans=$4),quality=$3)"

# Moving found files back to work directory
cd /tmp/planner_runs/$NEW_UUID 
tar cvf found_plans.tar found_plans && rm -rf found_plans && gzip found_plans.tar 
cd -
mv /tmp/planner_runs/$NEW_UUID/found_plans.tar.gz .
